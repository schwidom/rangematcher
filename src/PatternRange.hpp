#pragma once

#include "Pattern.hpp"

#include <memory>

class PatternRange
{
public:
 struct I
 {
  const std::shared_ptr<const Pattern> von;
  const std::shared_ptr<const Pattern> bis;
 };

 PatternRange( const I & i);

 const I & i() const ;

private:

 const I m_I;
};
