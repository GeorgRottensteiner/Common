#include <DX8/DX8Shader.h>

#include ".\guivbviewer.h"
#include "GUIComponentDisplayer.h"


CGUIVBViewer::CGUIVBViewer( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  CGUIScreen( iNewX, iNewY, iNewWidth, iNewHeight, dwId ),
  m_pVB( NULL ),
  m_pTexture( NULL ),
  m_vectDeltaRotate( 0, 0, 2 ),
  m_vectCurrentRotation( 0, 0, 0 )
{

  D3DXMATRIX    matWorld;

  D3DXMatrixIdentity( &m_matWorld );

}



void CGUIVBViewer::SetVertexBuffer( CVertexBuffer* pVB, CDX8Texture* pTexture )
{

  m_pVB         = pVB;
  m_pTexture    = pTexture;

}



COrientation& CGUIVBViewer::Orientation()
{

  return m_Orientation;

}



void CGUIVBViewer::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  if ( m_pVB )
  {
    Apply( *pViewer );
    pViewer->SetTexture( 0, m_pTexture );
    GR::CDX8ShaderFlat::Apply( *pViewer );

    // Projektions-Matrix anpassen, falls die Component halb angeschnitten ist
    D3DXMATRIX    matProj;

    GR::tRect   rcMyself;

    // der Viewport, wie er sein sollte
    GetWindowRect( rcMyself );

    GR::tPoint    ptCenter = rcMyself.center();

    float         fAspectRatio = (float)pViewer->Height() / (float)pViewer->Width();

    D3DXMatrixPerspectiveOffCenterLH( &matProj, 
              0.01f * 640.0f / rcMyself.width() * (float)( (int)pViewer->m_viewPort.X - ptCenter.x ),
              0.01f * 640.0f / rcMyself.width() * (float)( (int)pViewer->m_viewPort.Width + (int)pViewer->m_viewPort.X - ptCenter.x ),
              0.01f * 640.0f * fAspectRatio / rcMyself.height() * (float)( (int)pViewer->m_viewPort.Height + (int)pViewer->m_viewPort.Y - ptCenter.y ),
              0.01f * 640.0f * fAspectRatio / rcMyself.height() * (float)( (int)pViewer->m_viewPort.Y - ptCenter.y ),
                                      1.0f,
                                      50.0f );
    pViewer->SetTransform( D3DTS_PROJECTION, &matProj );

    D3DXMATRIX    matRotation;

    D3DXMatrixRotationYawPitchRoll( &matRotation, m_vectCurrentRotation.x, m_vectCurrentRotation.y, m_vectCurrentRotation.z );

    pViewer->SetTransform( D3DTS_WORLD, &( matRotation * m_matWorld ) );
    pViewer->SetTransform( D3DTS_VIEW, &m_Orientation.GetViewMatrix() );

    m_pVB->Display();
  }

}



void CGUIVBViewer::SetTransform( const D3DXMATRIX& matWorld )
{

  m_matWorld      = matWorld;

}



void CGUIVBViewer::Update( float fElapsedTime )
{

  m_vectCurrentRotation += m_vectDeltaRotate * fElapsedTime;

}