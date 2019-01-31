#pragma once

#include "MatchGot.hpp"

#include <memory>

#include <stdexcept> // logic_error

template<class T>
class Match {
public:

 virtual ~Match() = default;

 bool matched() const {
  return m_Matched;
 }

 std::shared_ptr<MatchGot<T> const> get() const
 {
  if( !matched()){
   throw std::logic_error("!matched");
  }
 
  return m_MatchGot;
 }

protected:

 bool m_Matched{false};

 std::shared_ptr<const MatchGot<T>> m_MatchGot{};
};
