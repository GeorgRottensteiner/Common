#ifndef _IRESOURCE_INCLUDED_
#define _IRESOURCE_INCLUDED_


#include <time.h>

#include <GR/GRTypes.h>



struct IResource
{
  time_t              m_LastAccessTime;

  GR::String          m_Name;

  size_t              m_Size,
                      m_References;

  bool                m_IsLoaded;



  IResource() :
    m_LastAccessTime( time( NULL ) ),
    m_Name( "" ),
    m_Size( 0 ),
    m_References( 1 ),
    m_IsLoaded( false )
  {
  }

  virtual ~IResource()
  {
  }

  virtual bool        Load() = 0;
  virtual bool        Release() = 0;
  virtual bool        Destroy() = 0;

  void                Touch()
  {
    m_LastAccessTime = time( NULL );
    if ( !IsLoaded() )
    {
      Load();
    }
  }

  size_t              Size() const
  {
    return m_Size;
  }

  bool                IsLoaded() const
  {
    return m_IsLoaded;
  }

};




#endif// _IRESOURCE_INCLUDED_

