#include "MatchRange.hpp"

MatchRange::MatchRange(const MatchRange::I & i)
: NamingWeakOrdered(i.namingWeakOrdered)
, m_I(i)
{
}

const MatchRange::I & MatchRange::i()
{
 return m_I;
}

