
#include "NamingWeakOrdered.hpp"

NamingWeakOrdered::NamingWeakOrdered( const NamingWeakOrdered::I & i)
: m_I( i)
{
}

const std::string & NamingWeakOrdered::getName() const 
{
 return m_I.name;
}

bool NamingWeakOrdered::operator<(const NamingWeakOrdered& that) const
{
 return m_I.name < that.getName();
}
