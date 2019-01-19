#pragma once

#include "NamedPatternRange.hpp"
#include "MatchRange.hpp"

#include <memory>

class NonOverlappingMatcher
{
public:
 NonOverlappingMatcher(const std::vector<std::shared_ptr<const NamedPatternRange>> & patternRange);

 std::unique_ptr<std::vector<MatchRange>> matchAll(Range range) const;

private:
 
 const std::vector<std::shared_ptr<const NamedPatternRange>> m_PatternRange;
};

