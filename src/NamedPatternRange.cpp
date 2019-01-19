#include "NamedPatternRange.hpp"

NamedPatternRange::NamedPatternRange(NamedPatternRange::I i)
: NamingWeakOrdered{std::move(i.namingWeakOrdered)}
, PatternRange{std::move(i.patternRange)} 
{
}

