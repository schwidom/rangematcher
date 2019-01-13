#pragma once

#include "Pattern.hpp"

#include <memory>

struct PatternRange
{
 const std::shared_ptr<const Pattern> von;
 const std::shared_ptr<const Pattern> bis;
};
