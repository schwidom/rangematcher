
#include "FileTo.hpp"

#include <fstream>
#include <stdexcept>

FileTo::FileTo( const std::string & filename)
: m_Filename{filename}
{
}


FileTo::operator std::unique_ptr<std::vector<char>> () const
{
 std::ifstream file(m_Filename, std::ios_base::binary | std::ios_base::in);

 if(!file) {
  throw std::logic_error("!file");
 }
 
 file.seekg(0, std::ios_base::end);
 
 std::ifstream::pos_type length{file.tellg()};

 file.seekg(0, std::ios_base::beg);
 
 std::unique_ptr<std::vector<char>> ret{ std::make_unique<std::vector<char>>(length)};
 
 file.read(ret->data(), length);

 return ret;
}



