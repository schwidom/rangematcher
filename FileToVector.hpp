#pragma once

#include <string>
#include <vector>
#include <memory>

class FileToVector
{
public:
 FileToVector( const std::string & filename);

 operator std::unique_ptr<std::vector<char>> () const ;

private:

 const std::string m_Filename;
};
