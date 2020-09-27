#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include "XMultiTexture2d.h"



XMultiTexture2d::XMultiTexture2d( X2dRenderer* pRenderer ) :
  m_pRenderer( pRenderer ),
  m_SplitWidth( 0 ),
  m_SplitHeight( 0 ),
  m_SplitTilesX( 0 ),
  m_SplitTilesY( 0 ),
  m_Filename( "" )
{
  Initialize( pRenderer );
}



XMultiTexture2d::~XMultiTexture2d()
{
  Clear();
}



void XMultiTexture2d::Initialize( X2dRenderer* pRenderer )
{
  m_pRenderer       = pRenderer;
}



bool XMultiTexture2d::LoadImage( GR::IEnvironment& Environment, const GR::String& FileName, int SplitWidth, int SplitHeight )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  ImageFormatManager*    pManager = (ImageFormatManager*)Environment.Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }

  GR::Graphic::ImageData*   pData = pManager->LoadData( FileName );

  if ( pData == NULL )
  {
    return false;
  }

  m_Filename      = FileName;

  m_SplitWidth    = SplitWidth;
  m_SplitHeight   = SplitHeight;

  m_SplitTilesX   = ( pData->Width() - 1 ) / m_SplitWidth + 1;
  m_SplitTilesY   = ( pData->Height() - 1 ) / m_SplitHeight + 1;

  for ( int i = 0; i < ( pData->Width() - 1 ) / m_SplitWidth + 1; ++i )
  {
    for ( int j = 0; j < ( pData->Height() - 1 ) / m_SplitHeight + 1; ++j )
    {
      int     iWidth  = m_SplitWidth;
      int     iHeight = m_SplitHeight;

      if ( i * m_SplitWidth + iWidth > pData->Width() )
      {
        iWidth = pData->Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + iHeight > pData->Height() )
      {
        iHeight = pData->Height() - j * m_SplitHeight;
      }
      XTexture*   pTexture = m_pRenderer->CreateTexture( iWidth, iHeight, pData->ImageFormat() );
      if ( pTexture == NULL )
      {
        delete pData;
        Clear();
        return false;
      }

      m_Textures.push_back( XTextureSection( pTexture, 0, 0, iWidth, iHeight ) );
    }
  }

  Restore( pData );
  delete pData;

  return true;
}



void XMultiTexture2d::Clear()
{
  if ( m_pRenderer )
  {
    tVectTextures::iterator   it( m_Textures.begin() );
    while ( it != m_Textures.end() )
    {
      XTextureSection& TexSec = *it;

      m_pRenderer->DestroyTexture( TexSec.m_pTexture );

      ++it;
    }
    m_Textures.clear();
  }
}



void XMultiTexture2d::Render2d( X2dRenderer& Renderer, int iX, int iY, const GR::u32 Color ) const
{
  for ( int i = 0; i < m_SplitTilesX; ++i )
  {
    for ( int j = 0; j < m_SplitTilesY; ++j )
    {
      const XTextureSection&  TexSec = m_Textures[i * m_SplitTilesY + j];

      if ( Color == 0xffffffff )
      {
        Renderer.RenderTextureSection( iX + i * m_SplitWidth,
                                       iY + j * m_SplitHeight,
                                       TexSec );
      }
      else
      {
        Renderer.RenderTextureSectionColorized( iX + i * m_SplitWidth,
                                                iY + j * m_SplitHeight,
                                                TexSec,
                                                Color );
      }
    }
  }
}



bool XMultiTexture2d::Restore( GR::Graphic::ImageData* pData )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  GR::Graphic::ContextDescriptor    cdSource;
  
  cdSource.Attach( pData->Width(), pData->Height(), pData->LineOffsetInBytes(), pData->ImageFormat(), pData->Data() );

  for ( int i = 0; i < ( pData->Width() - 1 ) / m_SplitWidth + 1; ++i )
  {
    for ( int j = 0; j < ( pData->Height() - 1 ) / m_SplitHeight + 1; ++j )
    {
      int     iWidth  = m_SplitWidth;
      int     iHeight = m_SplitHeight;

      if ( i * m_SplitWidth + iWidth > pData->Width() )
      {
        iWidth = pData->Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + iHeight > pData->Height() )
      {
        iHeight = pData->Height() - j * m_SplitHeight;
      }

      XTextureSection&    TexSec = m_Textures[i * m_SplitTilesY + j];

      GR::Graphic::ContextDescriptor      cdTarget;

      cdTarget.CreateData( iWidth, iHeight, pData->ImageFormat() );

      cdSource.CopyArea( i * m_SplitWidth, j * m_SplitHeight, iWidth, iHeight, 0, 0, &cdTarget );

      m_pRenderer->CopyDataToTexture( TexSec.m_pTexture, cdTarget );
    }
  }

  return true;
}



/*
bool XMultiTexture2d::ProcessEvent( const tXFrameEvent& Event )
{

  if ( Event.m_Type == tXFrameEvent::ET_DISPLAY_MODE_CHANGED )
  {
    CImageData*   pData = CImageFormatManager::Instance().LoadData( m_strFilename.c_str() );

    if ( pData == NULL )
    {
      return false;
    }
    Restore( pData );
    delete pData;
    return true;
  }
  else if ( Event.m_Type == tXFrameEvent::ET_SHUTDOWN )
  {
    Clear();
  }

  return false;

}
*/



bool XMultiTexture2d::Empty() const
{
  return m_Textures.empty();
}