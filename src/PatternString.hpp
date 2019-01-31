#pragma once

#include "Pattern.hpp"

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <string>
#include <vector>

#define TYPE std::vector<char>

class PatternString : public Pattern
{
private:

 class MatchString : public Match<TYPE>
 {
  public:
   MatchString( Range<TYPE> range, const std::shared_ptr<const std::string> pattern);

   virtual ~MatchString() override = default;
 };

public:

 PatternString( const std::string & pattern);

 virtual ~PatternString() override = default;

 std::unique_ptr<Match<TYPE>> match( Range<TYPE> range) const override;

private:

 const std::shared_ptr<const std::string> m_Pattern;
};

#undef TYPE

