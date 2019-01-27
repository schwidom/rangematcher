
#include "Console.hpp"

#include <iostream>

int main( int argc, char** argv)
{

 Console( StreamPair{std::cin, std::cout}); // TODO check default parameter, chech crash

 // Console *c = new Console( StreamPair{std::cin, std::cout}); // TODO check default parameter
 // delete c;

 // Console c( StreamPair{std::cin, std::cout}); // TODO check default parameter, check crash

 return 0;
}

