#pragma once

#include "Pattern.hpp"

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <string>
#include <vector>

class PatternString : public Pattern
{
private:

 class MatchString : public Match
 {
  public:
   MatchString( Range range, const std::shared_ptr<const std::string> pattern);

   virtual ~MatchString() override = default;
 };

public:

 PatternString( const std::string & pattern);

 virtual ~PatternString() override = default;

 std::unique_ptr<Match> match( Range range) const override;

private:

 const std::shared_ptr<const std::string> m_Pattern;
};
