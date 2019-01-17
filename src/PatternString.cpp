
#include "PatternString.hpp"

#include <algorithm> // std::search

// c++17:
// #include <functional> // boyer_moore_searcher

PatternString::PatternString( const std::string & pattern) 
 : m_Pattern{std::make_shared<const std::string>(pattern)}
{
}

std::unique_ptr<Match> PatternString::match( Range range) const {
 return std::make_unique<MatchString>(range, m_Pattern);
}

PatternString::MatchString::MatchString(Range range, const std::shared_ptr<const std::string> pattern) {
 auto itResult = std::search(range.begin, range.end, pattern->begin(), pattern->end());

 // c++17:
 // auto itResult = std::search(range.begin, range.end, std::boyer_moore_searcher(pattern->begin(), pattern->end()));
 // TODO : use memmem 59wbjcqqzn 

 if( true == ( m_Matched = ( itResult != range.end)))
 {
  m_MatchGot = std::make_shared<MatchGot>(Range{itResult, itResult + pattern->size()});
 }
}

