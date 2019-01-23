#pragma once

#include "FileTo.hpp"

class FileToVector
{
public:
 FileToVector( const std::string & filename);

 std::unique_ptr<std::vector<char>> get() const;

private:

 FileTo m_ToVector;

};
