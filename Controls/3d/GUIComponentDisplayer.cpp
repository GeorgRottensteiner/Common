#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"



GUIComponentDisplayer::GUIComponentDisplayer()
{
  m_iOffsetX    = 0;
  m_iOffsetY    = 0;
}



void GUIComponentDisplayer::SetClipping( int iX, int iY, int iWidth, int iHeight )
{
  D3DVIEWPORT8    viewPort = m_pViewer->m_viewPort;

  if ( iX < 0 )
  {
    iWidth += iX;
    iX = 0;
  }
  if ( iY < 0 )
  {
    iHeight += iY;
    iY = 0;
  }
  if ( iX + iWidth > m_pViewer->Width() )
  {
    iWidth = m_pViewer->Width() - iX;
  }
  if ( iY + iHeight >= m_pViewer->Height() )
  {
    iHeight = m_pViewer->Height() - iY;
  }

  if ( iX > m_pViewer->Width() )
  {
    iX = m_pViewer->Width();
    iWidth = 0;
  }
  if ( iY > m_pViewer->Height() )
  {
    iY = m_pViewer->Height();
    iHeight = 0;
  }
  if ( iWidth < 0 )
  {
    iWidth = 0;
  }
  if ( iHeight < 0 )
  {
    iHeight = 0;
  }

  viewPort.X = iX;
  viewPort.Y = iY;
  viewPort.Width = iWidth;
  viewPort.Height = iHeight;

  if ( ( iWidth == 0 )
  ||   ( iHeight == 0 ) )
  {
    m_bNothingIsVisible = true;
  }
  else
  {
    m_bNothingIsVisible = false;
    m_pViewer->SetViewport( &viewPort );
  }

  m_rectClipping.set( iX, iY, iWidth, iHeight );
  
}



void GUIComponentDisplayer::SetOffset( int iX, int iY )
{
  m_iOffsetX = iX;
  m_iOffsetY = iY;
}



void GUIComponentDisplayer::PushClipValues()
{
  StoreClipValues( m_rectClipping.Left, 
                   m_rectClipping.Top,
                   m_rectClipping.width(),
                   m_rectClipping.height(),
                   m_iOffsetX,
                   m_iOffsetY );
}



void GUIComponentDisplayer::DisplayAllControls()
{
  m_pViewer->SetRenderState( D3DRS_ZENABLE, FALSE );

  m_pViewer->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
  m_pViewer->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  m_pViewer->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  m_pViewer->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
  m_pViewer->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  m_pViewer->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
  m_pViewer->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  m_pViewer->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  m_rectClipping = GR::tRect( 0, 0, m_pViewer->Width(), m_pViewer->Height() );


  GUI::AbstractComponentDisplayer<GUIComponent>::DisplayAllControls();
}



void GUIComponentDisplayer::SetDefaultTextureSection( GR::u32 dwType, const tTextureSection& TexSection )
{
  if ( dwType >= 50 )
  {
    return;
  }
  while ( dwType >= m_DefaultTextureSection.size() )
  {
    m_DefaultTextureSection.push_back( tTextureSection() );
  }
  m_DefaultTextureSection[dwType] = TexSection;
}



GUIComponentDisplayer& GUIComponentDisplayer::Instance()
{
  static  GUIComponentDisplayer    g_Instance;

  return g_Instance;
}