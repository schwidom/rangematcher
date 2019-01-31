#pragma once

#include <vector>

template <class T> struct Range;

template <> struct Range<long> {

 using iterator = long;

 template <class T> Range( typename T::iterator iStart, Range<T> range)
  : begin{std::distance(iStart,range.begin)}
  , end{std::distance(iStart,range.end)}
 {
 }

 Range(iterator begin, iterator end) : begin{begin}, end{end} {}
 
 Range() : begin{0}, end{0} {}

 iterator begin;
 iterator end;
};

template <class T> struct Range
{
 using iterator = typename T::iterator;

 Range(T & t) : begin{t.begin()}, end{t.end()} {}
 Range(iterator begin, iterator end) : begin{begin}, end{end} {}

 Range(iterator begin, Range<long> range) : begin{begin}, end{begin}
 {
  std::advance(this->begin, range.begin);
  std::advance(end, range.end);
 }
 
 Range() : begin{0}, end{0} {}

 bool operator == (const Range<T> & range){ return begin == range.begin && end == range.end;}

 iterator begin;
 iterator end;
};


