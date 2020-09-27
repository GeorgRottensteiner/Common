#include "RefCountObject.h"



namespace GR
{

CRefCountObject::CRefCountObject() :
	m_refs(0)
{
}

CRefCountObject::CRefCountObject( const CRefCountObject& ) :
	m_refs(0)
{
}

CRefCountObject::~CRefCountObject()																		
{
}

CRefCountObject& CRefCountObject::operator=( const CRefCountObject& )
{
	return *this;
}

void CRefCountObject::addReference()
{
  m_refs++;
}



long CRefCountObject::removeReference()
{

  --m_refs;

  return m_refs;

}


void CRefCountObject::release()
{
  if ( 0 == --m_refs )
  {
		delete this;
  }
}



} // GR


