#pragma once

#include <vector>

struct Range {
 using iterator = std::vector<char>::iterator;
 iterator begin;
 iterator end;
};
