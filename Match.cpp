
#include "Match.hpp"

#include <stdexcept> // logic_error

bool Match::matched() const {
 return m_Matched;
}

std::shared_ptr<const MatchGot> Match::get() const
{
 if( !matched()){
  throw std::logic_error("!matched");
 }

 return m_MatchGot;
}

