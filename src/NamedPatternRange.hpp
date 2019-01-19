#pragma once

#include "NamingWeakOrdered.hpp"
#include "PatternRange.hpp"

class NamedPatternRange : public NamingWeakOrdered, public PatternRange {
public:
 struct I {
  NamingWeakOrdered::I namingWeakOrdered;
  PatternRange::I patternRange;
 };

 NamedPatternRange(I i);

};
