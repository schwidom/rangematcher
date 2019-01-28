#pragma once

#include <functional>
#include <map>
#include <memory>
#include <typeindex> // type_index(typeid())
#include <typeinfo> // typeid
#include <utility>

#include <stdexcept> // logic_error

template<class T>
class Any;

class AnyBase
{
public:
 virtual ~AnyBase() = default;
 
 template<class T> T* get()
 {
  Any<T> * self = dynamic_cast<Any<T> *>(this);

  if(self)
  {
   return &self->m_T;
  }
  else
  {

   const std::type_index typeIndex(typeid(T));

   auto itFound = m_TypeidNames2ValuesMap.find(typeIndex);

   if( itFound != m_TypeidNames2ValuesMap.end())
   {
    return static_cast<T*>( itFound->second.get()->get<T>());
   }

   return nullptr;
  }
 }

protected:

 std::map<std::type_index, std::unique_ptr<AnyBase>> m_TypeidNames2ValuesMap;
};

template<class T>
class Any : public AnyBase
{
public:
 explicit Any(T t)
 : m_T(t)
 , m_TypeIndex{std::type_index(typeid(T))}
 {
 }

 Any()
 : m_T()
 , m_TypeIndex{std::type_index(typeid(T))}
 {
 }

 template <class T2> void addType()
 {
  if( !m_T)
  {
   throw std::logic_error( "no value is set, conversion cannot be proved");
  }

  // auto tConverted = dynamic_cast<T2>(m_T);
  T2 tConverted( m_T); // implicit cast, good for shared_ptr

  if( ! tConverted)
  {
   throw std::logic_error( "no type conversion was possible");
  }

  const std::type_index typeIndex(typeid(T2));

  if( typeIndex == m_TypeIndex)
  {
   throw std::logic_error( "type is already known as primary type");
  }

  if( m_TypeidNames2ValuesMap.find(typeIndex) != m_TypeidNames2ValuesMap.end())
  {
   throw std::logic_error( "type is already registered");
  }

  // m_TypeidNames2ValuesMap.emplace(std::pair<std::type_index,std::unique_ptr<AnyBase>>(typeIndex, Any<T2>(tConverted))); // ?
  m_TypeidNames2ValuesMap[typeIndex] = std::make_unique<Any<T2>>(tConverted);
 }

private:
 T m_T;

 std::type_index m_TypeIndex;

 friend AnyBase;
};
