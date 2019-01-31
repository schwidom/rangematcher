
#include "PatternString.hpp"

#include <algorithm> // std::search

#define TYPE std::vector<char>

// c++17:
// #include <functional> // boyer_moore_searcher

PatternString::PatternString( const std::string & pattern) 
 : m_Pattern{std::make_shared<const std::string>(pattern)}
{
}

std::unique_ptr<Match<TYPE>> PatternString::match( Range<TYPE> range) const {
 return std::make_unique<MatchString>(range, m_Pattern);
}

PatternString::MatchString::MatchString(Range<TYPE> range, const std::shared_ptr<const std::string> pattern) {
 auto itResult = std::search(range.begin, range.end, pattern->begin(), pattern->end());

 // c++17:
 // auto itResult = std::search(range.begin, range.end, std::boyer_moore_searcher(pattern->begin(), pattern->end()));
 // TODO : use memmem 59wbjcqqzn 

 if( true == ( m_Matched = ( itResult != range.end)))
 {
  m_MatchGot = std::make_shared<MatchGot<TYPE>>(Range<TYPE>{itResult, itResult + pattern->size()});
 }
}

#undef TYPE

