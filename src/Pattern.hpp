#pragma once

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <vector>

#define TYPE std::vector<char>

class Pattern
{
public: 
 virtual ~Pattern() = default;

 virtual std::unique_ptr<Match<TYPE>> match( Range<TYPE> range) const = 0;

};

#undef TYPE

