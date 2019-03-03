#pragma once

#include "NamingWeakOrdered.hpp"
#include "Range.hpp"

template<class T>
class MatchRange : public NamingWeakOrdered
{
public:

 struct D
 {
  Range<T> begin;
  Range<T> end;
 };

 struct I
 {
  NamingWeakOrdered::I namingWeakOrdered;
  D d;
 };

 MatchRange( const I & i)
 : NamingWeakOrdered(i.namingWeakOrdered)
 , m_I(i)
 {
 }

 const MatchRange::I & i() const
 {
  return m_I;
 }


private:
 
 const I m_I{};
 
};
