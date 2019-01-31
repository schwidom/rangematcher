#include "NonOverlappingMatcher.hpp"

#include "Match.hpp"

#include <algorithm> // sort
#include <functional> // function

#include <stdexcept> // logic_error

#include <csignal>

#define TYPE std::vector<char>

NonOverlappingMatcher::NonOverlappingMatcher(const std::vector<std::shared_ptr<const NamedPatternRange>> & patternRange)
: m_PatternRange(patternRange)
{
}

namespace {

 struct Status {
  std::unique_ptr<Match<TYPE>> match;
 };

 using P = std::pair<NamedPatternRange const * , Status>; // s56plqg7n2 

 std::unique_ptr<Match<TYPE>> findOpeningMatch( std::vector<P> & v, Range<TYPE> range) 
 {
  for( auto & value : v) {

   if( ! value.second.match // pen45x1itp
    || ( value.second.match->matched() && value.second.match->get()->m_Range.begin < range.begin)
    ) // NOTE: this condition is an optimization, if in doubt, replace with true
   {
    std::unique_ptr<Match<TYPE>> match { value.first->i().von->match(range)};
    value.second.match = std::move(match);
   }
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

  return std::move( v.front().second.match); // pen45x1itp 
 }

 std::unique_ptr<Match<TYPE>> findClosingMatch( const std::vector<P> & v, Range<TYPE> range)
 {
  return v.at(0).first->i().bis->match(range);
 }
}

std::unique_ptr<std::vector<MatchRange<TYPE>>> NonOverlappingMatcher::matchAll(Range<TYPE> range) const
{
 std::vector<P> v;

 std::unique_ptr<std::vector<MatchRange<TYPE>>> ret{ std::make_unique<std::vector<MatchRange<TYPE>>>()};

 if( 0==m_PatternRange.size()){
  return std::move(ret); 
 }

 for( auto & namedPatternRange : m_PatternRange)
 {
  v.push_back(P(namedPatternRange.get(), Status{})); // pen45x1itp
 }

 Range<TYPE> currentRange(range);

 while( true)
 {

  // bicwwvbdnj 
  MatchRange<TYPE>::I matchRangeI{};

  // find first match

  std::unique_ptr<Match<TYPE>> matchOpen{ findOpeningMatch( v, currentRange)};

  if( !matchOpen->matched()){
   break; // no further matches
  }

  // bicwwvbdnj 
  matchRangeI.namingWeakOrdered = {v.at(0).first->getName()};
  matchRangeI.d.begin = matchOpen->get()->m_Range;
  matchRangeI.d.complete = false;
  matchRangeI.d.end = Range<TYPE>{ range.end, range.end};

  std::function<bool()> currentRangeCheck{ [&currentRange, &range]() -> bool {

   if( currentRange.begin == currentRange.end)
   {
    return false; // no further matches
   }

   if( currentRange.begin > currentRange.end)
   {
    throw std::logic_error("currentRange.begin > currentRange.end");
   }

   return true;
  }};

  currentRange.begin = matchOpen->get()->m_Range.end; // MatchGot

  if( ! currentRangeCheck()){
   ret->emplace_back(matchRangeI);
   break;
  }

  // seek closing match

  std::unique_ptr<Match<TYPE>> matchClose{ findClosingMatch( v, currentRange)};
  
  if( !matchClose->matched()){
   ret->emplace_back(matchRangeI);
   break; // no further matches
  }

  // bicwwvbdnj 
  matchRangeI.d.complete = true;
  matchRangeI.d.end = matchClose->get()->m_Range;

  currentRange.begin = matchClose->get()->m_Range.end; // MatchGot

  if( ! currentRangeCheck()){ 
   ret->emplace_back(matchRangeI);
   break;
  }

  ret->emplace_back(matchRangeI);
 }

 return std::move(ret);
}
