#pragma once

#include "MatchRange.hpp"
#include "NamedPatternRange.hpp"
#include "Range.hpp"

#include <memory>
#include <vector>

#define TYPE std::vector<char>

class NonOverlappingMatcher
{
public:
 NonOverlappingMatcher(const std::vector<std::shared_ptr<const NamedPatternRange>> & patternRange);

 std::unique_ptr<std::vector<MatchRange<TYPE>>> matchAll(Range<TYPE> range) const;

private:
 
 const std::vector<std::shared_ptr<const NamedPatternRange>> m_PatternRange;
};

#undef TYPE

