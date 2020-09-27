#ifndef _SOUND_RESOURCE_INCLUDED_
#define _SOUND_RESOURCE_INCLUDED_

#include <Interface/IResource.h>

class CSoundResource : public IResource
{

  public:

    DWORD             m_dwSoundHandle;

    GR::String       m_strFileName;

    bool              m_bLoadAs3d;


    CSoundResource( const GR::String& strFileName, bool bLoadAs3d = false ) :
      IResource( strFileName ),
      m_dwSoundHandle( 0xffffffff ),
      m_strFileName( strFileName ),
      m_bLoadAs3d( bLoadAs3d )
    {
    }

    virtual bool        Load()
    {
      if ( m_dwSoundHandle != -1 )
      {
        return true;
      }
      m_dwSoundHandle = CSoundSystem::Instance().LoadWave( m_strFileName.c_str(), m_bLoadAs3d );
      if ( m_dwSoundHandle == 0 )
      {
        m_dwSoundHandle = 0xffffffff;
        return false;
      }
      return true;
    }

    virtual bool        Release()
    {
      if ( m_dwSoundHandle == -1 )
      {
        return true;
      }
      CSoundSystem::Instance().ReleaseWave( m_dwSoundHandle );
      m_dwSoundHandle = -1;

      return true;
    }

    virtual bool        Destroy()
    {
      return Release();
    }

};

#endif// _SOUND_RESOURCE_INCLUDED_

