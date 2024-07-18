#include "RefCountObject.h"



namespace GR
{

  RefCountObject::RefCountObject() :
    m_References( 0 )
  {
  }



  RefCountObject::RefCountObject( const RefCountObject& ) :
    m_References( 0 )
  {
  }



  RefCountObject::~RefCountObject()																		
  {
  }



  RefCountObject& RefCountObject::operator=( const RefCountObject& )
  {
	  return *this;
  }



  void RefCountObject::AddReference()
  {
    ++m_References;
  }



  long RefCountObject::RemoveReference()
  {
    --m_References;

    return m_References;
  }


  void RefCountObject::Release()
  {
    if ( 0 == --m_References )
    {
		  delete this;
    }
  }



}


