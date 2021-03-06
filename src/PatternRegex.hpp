#pragma once

#include "Pattern.hpp"

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <regex>
#include <string>
#include <vector>

#define TYPE std::vector<char>

class PatternRegex : public Pattern
{
private:

 class MatchRegex : public Match<TYPE>
 {
  public:
   MatchRegex( Range<TYPE> range, const std::shared_ptr<const std::regex> regex);

   virtual ~MatchRegex() override = default;
 };

public:

 PatternRegex( const std::shared_ptr<const std::string> pattern);

 PatternRegex( const std::string & pattern);

 virtual ~PatternRegex() override = default;

 std::unique_ptr<Match<TYPE>> match( Range<TYPE> range) const override;

private:

 const std::shared_ptr<const std::string> m_Pattern;
 const std::shared_ptr<const std::regex> m_Regex;
};

#undef TYPE
