
#include "PatternVector.hpp"

#include <algorithm> // std::search

#include <iostream>

#define TYPE std::vector<char>

PatternVector::PatternVector( const std::shared_ptr<const std::vector<char>> pattern)
 : m_Pattern{std::move(pattern)}
{
}

PatternVector::PatternVector( const std::vector<char> & pattern)
 : m_Pattern{std::make_shared<const std::vector<char>>(pattern)}
{
}

std::unique_ptr<Match<TYPE>> PatternVector::match( Range<TYPE> range) const {
 return std::make_unique<MatchVector>(range, m_Pattern);
}

PatternVector::MatchVector::MatchVector(Range<TYPE> range, const std::shared_ptr<const std::vector<char>> pattern) {
 auto itResult = std::search(range.begin, range.end, pattern->begin(), pattern->end());

 // TODO : use strstr 59wbjcqqzn 

 if( true == ( m_Matched = ( itResult != range.end)))
 {
  m_MatchGot = std::make_shared<MatchGot<TYPE>>(Range<TYPE>{itResult, itResult + pattern->size()});
 }
}

#undef TYPE

