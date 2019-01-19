#pragma once

#include "NamedPatternRange.hpp"

#include <memory>

class NonOverlappingMatcher
{
public:
 NonOverlappingMatcher(std::vector<std::shared_ptr<NamedPatternRange>> patternRange);

 void matchAll(Range range);

private:
 
 std::vector<std::shared_ptr<NamedPatternRange>> m_PatternRange;
};

