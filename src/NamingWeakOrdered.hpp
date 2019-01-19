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
 virtual ~NamingWeakOrdered() = default;

 const std::string & getName() const ;

 bool operator<(const NamingWeakOrdered& that) const;

private:

 const I m_I;

};
