#pragma once

#include "Match.hpp"
#include "Range.hpp"

#include <memory>
#include <vector>

class Pattern
{
public: 
 virtual ~Pattern() = default;

 virtual std::unique_ptr<Match> match( Range range) const = 0;

};

