#pragma once

#include "MatchRange.hpp"
#include "NamedPatternRange.hpp"
#include "Range.hpp"

#include <memory>
#include <tuple>
#include <vector>

#define TYPE std::vector<char>

class NonOverlappingMatcher
{
public:
 NonOverlappingMatcher(const std::vector<std::shared_ptr<const NamedPatternRange>> & patternRange);

 /**
  * @brief matchAll
  * @returns a tuple of a boolean which denotes whether the last match is complete or not and a vector of matches
  */
 std::tuple<bool,std::unique_ptr<std::vector<MatchRange<TYPE>>>> matchAll(Range<TYPE> range) const;

private:
 
 const std::vector<std::shared_ptr<const NamedPatternRange>> m_PatternRange;
};

#undef TYPE

