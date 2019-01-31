#pragma once

#include "Range.hpp"

#include <vector>

template<class T>
class MatchGot {
public:
 MatchGot( Range<T> range) : m_Range(range) { }

 Range<T> m_Range;
};
