#pragma once

#include "NamingWeakOrdered.hpp"
#include "Range.hpp"

class MatchRange : public NamingWeakOrdered
{
public:

 struct D
 {
  Range begin;
  bool complete;
  Range end;
 };

 struct I
 {
  NamingWeakOrdered::I namingWeakOrdered;
  D d;
 };

 MatchRange( const I & i);

 const I & i();

private:
 
 const I m_I{};
 
};
