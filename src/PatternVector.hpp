#pragma once

#include "Pattern.hpp"

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <string>
#include <vector>

class PatternVector : public Pattern
{
private:

 class MatchVector : public Match
 {
  public:
   MatchVector( Range range, const std::shared_ptr<const std::vector<char>> pattern);

   virtual ~MatchVector() override = default;
 };

public:

 PatternVector( const std::shared_ptr<const std::vector<char>> pattern);

 PatternVector( const std::vector<char> & pattern);

 virtual ~PatternVector() override = default;

 std::unique_ptr<Match> match( Range range) const override;

private:

 const std::shared_ptr<const std::vector<char>> m_Pattern;
};
