#include "XHugeTexture.h"

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Grafik/ContextDescriptor.h>



XHugeTexture::XHugeTexture( XRenderer* pRenderer, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer ) :
  m_pRenderer( pRenderer ),
  m_pEventProducer( pEventProducer ),
  m_SplitWidth( 0 ),
  m_SplitHeight( 0 ),
  m_SplitTilesX( 0 ),
  m_SplitTilesY( 0 ),
  m_Filename( "" )
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



void XHugeTexture::Initialize( XRenderer* pRenderer, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer )
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



bool XHugeTexture::LoadImage( const char* FileName, int SplitWidth, int SplitHeight )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  GR::Graphic::ImageData*   pData = ImageFormatManager::Instance().LoadData( FileName );
  if ( pData == NULL )
  {
    return false;
  }

  m_Filename = FileName;

  if ( !SetImage( *pData, SplitWidth, SplitHeight ) )
  {
    return false;
  }
  delete pData;
  return true;
}



bool XHugeTexture::SetImage( GR::Graphic::ImageData& Image, int SplitWidth, int SplitHeight )
{
  if ( m_pRenderer == NULL )
  {
    return false;
  }

  m_Filename.clear();

  m_SplitWidth   = SplitWidth;
  m_SplitHeight  = SplitHeight;

  m_SplitTilesX = ( Image.Width() - 1 ) / m_SplitWidth + 1;
  m_SplitTilesY = ( Image.Height() - 1 ) / m_SplitHeight + 1;

  for ( int i = 0; i < ( Image.Width() - 1 ) / m_SplitWidth + 1; ++i )
  {
    for ( int j = 0; j < ( Image.Height() - 1 ) / m_SplitHeight + 1; ++j )
    {
      int     width  = m_SplitWidth;
      int     height = m_SplitHeight;

      if ( i * m_SplitWidth + width > Image.Width() )
      {
        width = Image.Width() - i * m_SplitWidth;
      }
      if ( j * m_SplitHeight + height > Image.Height() )
      {
        height = Image.Height() - j * m_SplitHeight;
      }
      XTexture*   pTexture = m_pRenderer->CreateTexture( width, height, Image.ImageFormat() );
      if ( pTexture == NULL )
      {
        Clear();
        return false;
      }
      m_Textures.push_back( XTextureSection( pTexture, 0, 0, width, height ) );
    }
  }

  Restore( &Image );

  return true;
}



void XHugeTexture::Clear()
{
  if ( m_pRenderer )
  {
    tTextures::iterator   it( m_Textures.begin() );
    while ( it != m_Textures.end() )
    {
      XTextureSection& ts( *it );

      m_pRenderer->DestroyTexture( ts.m_pTexture );

      ++it;
    }
    m_Textures.clear();
  }
}



void XHugeTexture::Render2d( XRenderer& Renderer, int X, int Y, GR::u32 Color )
{
  for ( int i = 0; i < m_SplitTilesX; ++i )
  {
    for ( int j = 0; j < m_SplitTilesY; ++j )
    {
      if ( i * m_SplitTilesY + j < (int)m_Textures.size() )
      {
        XTextureSection&  ts( m_Textures[i * m_SplitTilesY + j] );

        Renderer.RenderTextureSection2d( X + i * m_SplitWidth,
                                         Y + j * m_SplitHeight,
                                         ts,
                                         Color );
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

      XTextureSection&    ts( m_Textures[i * m_SplitTilesY + j] );

      GR::Graphic::ContextDescriptor      cdTarget;

      cdTarget.CreateData( ts.m_pTexture->m_SurfaceSize.x, ts.m_pTexture->m_SurfaceSize.y, pData->ImageFormat() );

      cdSource.CopyArea( i * m_SplitWidth, j * m_SplitHeight, width, height, 0, 0, &cdTarget );

      m_pRenderer->CopyDataToTexture( ts.m_pTexture, cdTarget );
    }
  }
  return true;
}



bool XHugeTexture::ProcessEvent( const GR::Gamebase::tXFrameEvent& Event )
{
  if ( Event.m_Type == GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED )
  {
    /*
    GR::Graphic::ImageData*   pData = ImageFormatManager::Instance().LoadData( m_Filename.c_str() );

    if ( pData == NULL )
    {
      return false;
    }
    Restore( pData );
    delete pData;
    */
    return true;
  }
  else if ( Event.m_Type == GR::Gamebase::tXFrameEvent::ET_SHUTDOWN )
  {
    Clear();
    m_pRenderer = NULL;
    m_pEventProducer = NULL;
  }
  return false;
}