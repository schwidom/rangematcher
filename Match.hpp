#pragma once

#include "MatchGot.hpp"

#include <memory>

class Match {
public:

 virtual ~Match() = default;

 bool matched() const;

 std::shared_ptr<MatchGot const> get() const; 

protected:

 bool m_Matched{false};

 std::shared_ptr<const MatchGot> m_MatchGot{};
};
