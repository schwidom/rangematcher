
#include "PatternRegex.hpp"

#include <algorithm> // std::search
#include <iterator> // std::advance

PatternRegex::PatternRegex( const std::shared_ptr<const std::string> pattern)
 : m_Pattern{std::move(pattern)}
 , m_Regex{std::make_shared<const std::regex>(*m_Pattern)}
{
}

PatternRegex::PatternRegex( const std::string & pattern)
 : m_Pattern{std::make_shared<const std::string>(pattern)}
 , m_Regex{std::make_shared<const std::regex>(*m_Pattern)}
{
}

std::unique_ptr<Match> PatternRegex::match( Range range) const {
 return std::make_unique<MatchRegex>(range, m_Regex);
}

PatternRegex::MatchRegex::MatchRegex(Range range, const std::shared_ptr<const std::regex> regex) {
 // auto itResult = std::search(range.begin, range.end, pattern->begin(), pattern->end());

 std::match_results<Range::iterator> match; // cppman match_results

 if( true == ( m_Matched = ( std::regex_search(range.begin, range.end, match, *regex))))
 {
  // m_MatchGot = std::make_shared<MatchGot>(Range{match.begin(), match.end()}); // TODO

  Range res{ range.begin, range.begin};

  std::advance( res.begin, match.position());
  std::advance( res.end, match.position() + match.length());

  m_MatchGot = std::make_shared<MatchGot>(res);
 }
}

