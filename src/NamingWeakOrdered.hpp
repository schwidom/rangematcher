#pragma once

#include <string>

class NamingWeakOrdered
{
public:

 struct I
 {
  std::string name;
 };

 explicit NamingWeakOrdered(const I & i);

 const std::string & getName() const ;

 bool operator<(const NamingWeakOrdered& that) const;

private:

 const I m_I;

};
