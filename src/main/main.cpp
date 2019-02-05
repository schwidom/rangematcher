
#include "Console.hpp"

#include <iostream>

int main( int argc, char** argv)
{

 Console( StreamPair{std::cin, std::cout}); 

 //! WARNING: a crash with a dubious stacktrace can happen if the project 
 //!  is not proper compiled and linked (with outdated modules)

 return 0;
}

