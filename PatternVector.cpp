
#include "PatternVector.hpp"

#include <algorithm> // std::search

#include <iostream>

PatternVector::PatternVector( const std::shared_ptr<const std::vector<char>> pattern)
 : m_Pattern{std::move(pattern)}
{
}

PatternVector::PatternVector( const std::vector<char> & pattern)
 : m_Pattern{std::make_shared<const std::vector<char>>(pattern)}
{
}

std::unique_ptr<Match> PatternVector::match( Range range) const {
 return std::make_unique<MatchVector>(range, m_Pattern);
}

PatternVector::MatchVector::MatchVector(Range range, const std::shared_ptr<const std::vector<char>> pattern) {
 auto itResult = std::search(range.begin, range.end, pattern->begin(), pattern->end());

 // TODO : use strstr 59wbjcqqzn 

 if( true == ( m_Matched = ( itResult != range.end)))
 {
  m_MatchGot = std::make_shared<MatchGot>(Range{itResult, itResult + pattern->size()});
 }
}

