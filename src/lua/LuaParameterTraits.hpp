#pragma once

#include <tuple>

template <class ... T> struct CallingParameter
{
 using type = std::tuple<T...>;
};

template <class ... T> struct ReturningParameter
{
 using type = std::tuple<T...>;
};


template <class T> struct P; 
template <class ... T> struct V; 

// NOTE: P<T> represents lua parameters of type T which are to be used inside the functions
// NOTE: V<T> represents parameters of type T which are represented by descriptors referring to "vectorOfObjects"
// NOTE: V<T> ... in case of returning the parameter gets saved and the descriptor is returned

