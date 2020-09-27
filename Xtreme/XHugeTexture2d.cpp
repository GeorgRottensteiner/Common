#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Grafik/ContextDescriptor.h>

#include "X2dRenderer.h"

#include "XHugeTexture2d.h"



XHugeTexture::XHugeTexture( X2dRenderer* pRenderer, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer ) :
  m_pRenderer( pRenderer ),
  m_pEventProducer( NULL ),
  m_SplitWidth( 0 ),
  m_SplitHeight( 0 ),
  m_SplitTilesX( 0 ),
  m_SplitTilesY( 0 ),
  m_Filename( "" ),
  m_ColorKey( 0 ),
  m_ColorKeyed( false ),
  m_ColorizeColor( -1 )
{

  Initialize( pRenderer, pEventProducer );

}



XHugeTexture::~XHugeTexture()
{

  Clear();

  if ( m_pEventProducer )
  {
    m_pEventProducer->RemoveListener( this );
  }

}



void XHugeTexture::Initialize( X2dRenderer* pRenderer, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer )
{

  if ( m_pEventProducer )
  {
    m_pEventProducer->RemoveListener( this );
  }
  m_pRenderer       = pRenderer;
  m_pEventProducer  = pEventProducer;
  if ( m_pEventProducer )
  {
    m_pEventProducer->AddListener( this );
  }

}



bool XHugeTexture::LoadImage( const char* FileName, int SplitWidth, int SplitHeight, GR::u32 ColorKey, GR::u32 ColorizeColor )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  m_ImageSource = GR::Graphic::ImageData();
  GR::Graphic::ImageData*   pData = ImageFormatManager::Instance().LoadData( FileName );
  if ( pData == NULL )
  {
    return false;
  }

  if ( ColorKey != 0 )
  {
    pData->ConvertSelfTo( pData->ImageFormat(), 0, true, ColorKey );
    m_ColorKey = ColorKey;
    m_ColorKeyed = true;
  }

  m_Filename      = FileName;

  m_ColorizeColor = ColorizeColor;

  m_SplitWidth    = SplitWidth;
  m_SplitHeight   = SplitHeight;

  m_SplitTilesX   = ( pData->Width() - 1 ) / m_SplitWidth + 1;
  m_SplitTilesY   = ( pData->Height() - 1 ) / m_SplitHeight + 1;

  for ( int i = 0; i < ( pData->Width() - 1 ) / m_SplitWidth + 1; ++i )
  {
    for ( int j = 0; j < ( pData->Height() - 1 ) / m_SplitHeight + 1; ++j )
    {
      int     width  = m_SplitWidth;
      int     height = m_SplitHeight;

      if ( i * m_SplitWidth + width > pData->Width() )
      {
        width = pData->Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + height > pData->Height() )
      {
        height = pData->Height() - j * m_SplitHeight;
      }
      XTexture*   pTexture = m_pRenderer->CreateTexture( width, height, pData->ImageFormat() );
      if ( pTexture == NULL )
      {
        delete pData;
        Clear();
        return false;
      }

      m_Textures.push_back( XTextureSection( pTexture, 0, 0, width, height ) );
    }
  }

  Restore( pData );
  delete pData;

  return true;

}



bool XHugeTexture::LoadImageData( GR::Graphic::ImageData& ImageData, int SplitWidth, int SplitHeight, GR::u32 ColorKey, GR::u32 ColorizeColor )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  m_Filename.clear();

  m_ImageSource = GR::Graphic::ImageData( ImageData );

  GR::Graphic::ImageData    tempData( m_ImageSource );

  if ( ColorKey != 0 )
  {
    tempData.ConvertSelfTo( tempData.ImageFormat(), 0, true, ColorKey );
    m_ColorKey = ColorKey;
    m_ColorKeyed = true;
  }

  m_ColorizeColor = ColorizeColor;

  m_SplitWidth = SplitWidth;
  m_SplitHeight = SplitHeight;

  m_SplitTilesX = ( tempData.Width() - 1 ) / m_SplitWidth + 1;
  m_SplitTilesY = ( tempData.Height() - 1 ) / m_SplitHeight + 1;

  for ( int i = 0; i < ( tempData.Width() - 1 ) / m_SplitWidth + 1; ++i )
  {
    for ( int j = 0; j < ( tempData.Height() - 1 ) / m_SplitHeight + 1; ++j )
    {
      int     width = m_SplitWidth;
      int     height = m_SplitHeight;

      if ( i * m_SplitWidth + width > tempData.Width() )
      {
        width = tempData.Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + height > tempData.Height() )
      {
        height = tempData.Height() - j * m_SplitHeight;
      }
      XTexture*   pTexture = m_pRenderer->CreateTexture( width, height, tempData.ImageFormat() );
      if ( pTexture == NULL )
      {
        Clear();
        return false;
      }

      m_Textures.push_back( XTextureSection( pTexture, 0, 0, width, height ) );
    }
  }

  Restore( &tempData );

  return true;
}



void XHugeTexture::Clear()
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
  }
  m_Textures.clear();
  m_Filename.erase();
  m_ImageSource = GR::Graphic::ImageData();
}



void XHugeTexture::Render2d( X2dRenderer& Renderer, int X, int Y )
{
  if ( m_ColorKeyed )
  {
    Render2dColorKeyed( Renderer, m_ColorKey, X, Y );
    return;
  }
  for ( int i = 0; i < m_SplitTilesX; ++i )
  {
    for ( int j = 0; j < m_SplitTilesY; ++j )
    {
      XTextureSection&  TexSec = m_Textures[i * m_SplitTilesY + j];

      if ( m_ColorizeColor != -1 )
      {
        Renderer.RenderTextureSectionColorized( X + i * m_SplitWidth,
                                                Y + j * m_SplitHeight,
                                                TexSec,
                                                m_ColorizeColor );
      }
      else
      {
        Renderer.RenderTextureSection( X + i * m_SplitWidth,
                                       Y + j * m_SplitHeight,
                                       TexSec );
      }
    }
  }
}



void XHugeTexture::Render2dColorKeyed( X2dRenderer& Renderer, GR::u32 ColorKey, int X, int Y )
{
  for ( int i = 0; i < m_SplitTilesX; ++i )
  {
    for ( int j = 0; j < m_SplitTilesY; ++j )
    {
      XTextureSection&  TexSec = m_Textures[i * m_SplitTilesY + j];

      if ( m_ColorizeColor != -1 )
      {
        Renderer.RenderTextureSectionColorKeyedColorized( X + i * m_SplitWidth,
                                                          Y + j * m_SplitHeight,
                                                          TexSec,
                                                          ColorKey,
                                                          m_ColorizeColor );
      }
      else
      {
        Renderer.RenderTextureSectionColorKeyed( X + i * m_SplitWidth,
                                                 Y + j * m_SplitHeight,
                                                 TexSec,
                                                 ColorKey );
      }
    }
  }
}



bool XHugeTexture::Restore( GR::Graphic::ImageData* pData )
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
      int     width  = m_SplitWidth;
      int     height = m_SplitHeight;

      if ( i * m_SplitWidth + width > pData->Width() )
      {
        width = pData->Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + height > pData->Height() )
      {
        height = pData->Height() - j * m_SplitHeight;
      }

      XTextureSection&    TexSec = m_Textures[i * m_SplitTilesY + j];

      GR::Graphic::ContextDescriptor      cdTarget;

      cdTarget.CreateData( width, height, pData->ImageFormat() );

      cdSource.CopyArea( i * m_SplitWidth, j * m_SplitHeight, width, height, 0, 0, &cdTarget );

      m_pRenderer->CopyDataToTexture( TexSec.m_pTexture, cdTarget );
    }
  }
  return true;
}



bool XHugeTexture::ProcessEvent( const GR::Gamebase::tXFrameEvent& Event )
{

  if ( Event.m_Type == GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED )
  {
    if ( !m_Filename.empty() )
    {
      GR::Graphic::ImageData*   pData = ImageFormatManager::Instance().LoadData( m_Filename.c_str() );

      if ( pData == NULL )
      {
        return false;
      }
      Restore( pData );
      delete pData;
    }
    Restore( &m_ImageSource );
    return true;
  }
  else if ( Event.m_Type == GR::Gamebase::tXFrameEvent::ET_SHUTDOWN )
  {
    Clear();
  }
  return false;
}



bool XHugeTexture::Empty() const
{
  return m_Textures.empty();
}