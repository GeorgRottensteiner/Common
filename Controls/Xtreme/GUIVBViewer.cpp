#include "GUIVBViewer.h"
#include "GUIComponentDisplayer.h"


GUIVBViewer::GUIVBViewer( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  GUIScreen( iNewX, iNewY, iNewWidth, iNewHeight, dwId ),
  m_pVB( NULL ),
  m_pTexture( NULL ),
  m_vectDeltaRotate( 0, 0, 2 ),
  m_vectCurrentRotation( 0, 0, 0 )
{
  m_matWorld.Identity();
}



void GUIVBViewer::SetVertexBuffer( XVertexBuffer* pVB, XTexture* pTexture )
{

  m_pVB         = pVB;
  m_pTexture    = pTexture;

}



XOrientation& GUIVBViewer::Orientation()
{

  return m_Orientation;

}



void GUIVBViewer::DisplayOnPage( XRenderer* pRenderer, int iOffsetX, int iOffsetY )
{

  if ( m_pVB )
  {
    // TODO - Renderstates?
    //Apply( *pViewer );
    pRenderer->SetTexture( 0, m_pTexture );

    pRenderer->SetShader( XRenderer::ST_FLAT );

    // Projektions-Matrix anpassen, falls die Component halb angeschnitten ist
    math::matrix4   matProj;

    GR::tRect   rcMyself;

    // der Viewport, wie er sein sollte
    GetWindowRect( rcMyself );

    GR::tPoint    ptCenter = rcMyself.Center();

    float         fAspectRatio = (float)pRenderer->Height() / (float)pRenderer->Width();

    // TODO
    /*
    D3DXMatrixPerspectiveOffCenterLH( &matProj, 
              0.01f * 640.0f / rcMyself.width() * (float)( (int)pViewer->m_viewPort.X - ptCenter.x ),
              0.01f * 640.0f / rcMyself.width() * (float)( (int)pViewer->m_viewPort.Width + (int)pViewer->m_viewPort.X - ptCenter.x ),
              0.01f * 640.0f * fAspectRatio / rcMyself.height() * (float)( (int)pViewer->m_viewPort.Height + (int)pViewer->m_viewPort.Y - ptCenter.y ),
              0.01f * 640.0f * fAspectRatio / rcMyself.height() * (float)( (int)pViewer->m_viewPort.Y - ptCenter.y ),
                                      1.0f,
                                      50.0f );
    pViewer->SetTransform( D3DTS_PROJECTION, &matProj );
    */

    math::matrix4    matRotation;

    // TODO
    /*
    D3DXMatrixRotationYawPitchRoll( &matRotation, m_vectCurrentRotation.x, m_vectCurrentRotation.y, m_vectCurrentRotation.z );

    pViewer->SetTransform( D3DTS_WORLD, &( matRotation * m_matWorld ) );
    */
    pRenderer->SetTransform( XRenderer::TT_VIEW, m_Orientation.GetViewMatrix() );

    pRenderer->RenderVertexBuffer( m_pVB );
  }

}



void GUIVBViewer::SetTransform( const math::matrix4& matWorld )
{

  m_matWorld      = matWorld;

}



void GUIVBViewer::Update( float fElapsedTime )
{

  m_vectCurrentRotation += m_vectDeltaRotate * fElapsedTime;

}