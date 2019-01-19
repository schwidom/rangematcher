#include "PatternRange.hpp"

PatternRange::PatternRange( const PatternRange::I & i)
: m_I( i)
{
}

const PatternRange::I & PatternRange::i() const 
{
 return m_I;
}


