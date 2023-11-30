#pragma once


#include <map>

#include <GR/GRTypes.h>

#include <Interface/ICloneAble.h>



class CloneFactory
{

  public:

    typedef ICloneAble* (*CloneFunc)(); 

    typedef std::map<GR::String,CloneFunc>  tClones;

    tClones                   m_Clones;


  public:

    ~CloneFactory();

    void                      Clear();

    ICloneAble*               CreateObject( const GR::String& Name );
    bool                      RegisterObject( const GR::String& Name, CloneFunc Func, bool ReplaceExisting = true );

    static CloneFactory&      Instance();



    template <typename T> T* Cast( ICloneAble* pClone )
    {

      if ( pClone == NULL )
      {
        return NULL;
      }

      T   Dummy;

      auto it( m_Clones.find( Dummy.Class() ) );
      if ( it == m_Clones.end() )
      {
        return NULL;
      }

      if ( pClone->Class() == it->first )
      {
        return (T*)pClone;
      }
      return NULL;

    }

};


