#include <debug\debugclient.h>

#include "ImageFormatManager.h"



const tMapFormate& ImageFormatManager::GetFormatMap() const
{

  return m_mapFormate;

}



GR::String ImageFormatManager::IdentifyType( const GR::String& FileName )
{
  tMapFormate::iterator   it( m_mapFormate.begin() );
  while ( it != m_mapFormate.end() )
  {
    AbstractImageFileFormat*  pFormat = it->second;

    if ( pFormat->IsFileOfType( FileName ) )
    {
      return pFormat->GetDescription();
    }

    ++it;
  }
  return "Unknown";
}



GR::Graphic::ImageData* ImageFormatManager::LoadData( const GR::String& FileName, AbstractImageFileFormat** pPlugin )
{
  tMapFormate::iterator   it( m_mapFormate.begin() );
  while ( it != m_mapFormate.end() )
  {
    AbstractImageFileFormat*  pFormat = it->second;

    if ( pFormat->IsFileOfType( FileName ) )
    {
      GR::Graphic::ImageData* pData = pFormat->Load( FileName );
      if ( pData )
      {
        if ( pPlugin != NULL )
        {
          *pPlugin = pFormat;
        }
        return pData;
      }
    }

    ++it;
  }

  return NULL;
}



ImageSet* ImageFormatManager::LoadSet( const GR::String& FileName, AbstractImageFileFormat** pPlugin )
{
  tMapFormate::iterator   it( m_mapFormate.begin() );
  while ( it != m_mapFormate.end() )
  {
    AbstractImageFileFormat*  pFormat = it->second;

    if ( pFormat->IsFileOfType( FileName ) )
    {
      ImageSet* pSet = pFormat->LoadSet( FileName );
      if ( pSet )
      {
        if ( pPlugin != NULL )
        {
          *pPlugin = pFormat;
        }
        return pSet;
      }
    }
    ++it;
  }
  return NULL;
}



bool ImageFormatManager::SaveData( const GR::String& FileName, GR::Graphic::ImageData* pData )
{
  if ( pData == NULL )
  {
    return false;
  }

  tMapFormate::iterator   it( m_mapFormate.begin() );
  while ( it != m_mapFormate.end() )
  {
    AbstractImageFileFormat*  pFormat = it->second;

    if ( pFormat->CanSave( pData ) )
    {
      if ( pFormat->Save( FileName, pData ) )
      {
        return true;
      }
    }
    ++it;
  }
  return false;
}




ImageFormatManager& ImageFormatManager::Instance()
{ 
  static ImageFormatManager    LocalInstance;

  return LocalInstance;
}



