#include "FileToVector.hpp"

FileToVector::FileToVector( const std::string & filename)
: m_ToVector{filename}
{
}

std::unique_ptr<std::vector<char>> FileToVector::get() const
{
 return static_cast<std::unique_ptr<std::vector<char>>>(m_ToVector);
}
