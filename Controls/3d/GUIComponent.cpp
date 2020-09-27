#include <Debug\debugclient.h>

#include <DX8/DX8Shader.h>

#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"

#include <DX8\DX8Font.h>



GUIComponent::GUIComponent( GR::u32 dwId ) :
  GUI::Component( dwId )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  memcpy( &m_TextureSection, &GUIComponentDisplayer::Instance().m_DefaultTextureSection[0], GUIComponentDisplayer::Instance().m_DefaultTextureSection.size() * sizeof( tTextureSection ) );
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int iX, int iY, int iWidth, int iHeight, GR::u32 dwId ) :
  GUI::Component( iX, iY, iWidth, iHeight, dwId )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  memcpy( &m_TextureSection, &GUIComponentDisplayer::Instance().m_DefaultTextureSection[0], GUIComponentDisplayer::Instance().m_DefaultTextureSection.size() * sizeof( tTextureSection ) );
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int iX, int iY, int iWidth, int iHeight, const char* szCaption, GR::u32 dwId ) :
  GUI::Component( iX, iY, iWidth, iHeight, szCaption, dwId )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  memcpy( &m_TextureSection, &GUIComponentDisplayer::Instance().m_DefaultTextureSection[0], GUIComponentDisplayer::Instance().m_DefaultTextureSection.size() * sizeof( tTextureSection ) );
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



void GUIComponent::Display( GUI::AbstractComponentDisplayer<GUIComponent>* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;
  DisplayOnPage( pCompDisplayer->m_pViewer, pCompDisplayer->m_iOffsetX, pCompDisplayer->m_iOffsetY );
}



void GUIComponent::DisplayNonClient( GUI::AbstractComponentDisplayer<GUIComponent>* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;
  DisplayNonClientOnPage( pCompDisplayer->m_pViewer, pCompDisplayer->m_iOffsetX, pCompDisplayer->m_iOffsetY );
}



void GUIComponent::DisplayNonClientOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{
  GR::CDX8ShaderAlphaBlendAndTest::Apply( *pViewer );

  if ( !( EdgeStyle() & GUI::GET_TRANSPARENT_BKGND ) )
  {
    GR::tRect  rcClient;

    GR::tPoint ptClientOffset = GetClientOffset();


    GetClientRect( rcClient );
    
    if ( m_TextureSection[GUI::BT_BACKGROUND].m_pTexture )
    {
      pViewer->DrawTextureSection( iOffsetX + ptClientOffset.x, 
                                  iOffsetY + ptClientOffset.y, 
                                  m_TextureSection[GUI::BT_BACKGROUND], 0xffffffff, 
                                  rcClient.width(),
                                  rcClient.height() );
    }
    else
    {
      pViewer->DrawBox( iOffsetX + ptClientOffset.x, 
                        iOffsetY + ptClientOffset.y, 
                        rcClient.width(),
                        rcClient.height(),
                        GetColor( GUI::COL_BTNFACE ) );
    }
  }

  GR::tRect   rectComp;
  GetComponentRect( rectComp );

  rectComp.offset( iOffsetX, iOffsetY );
  DrawEdge( pViewer, m_Edge, rectComp );
}



void GUIComponent::DrawEdge( CD3DViewer* pViewer, GR::u32 edgeType, GR::tRect& rectEdge )
{
  int   iX1 = rectEdge.position().x,
        iY1 = rectEdge.position().y,
        iX2 = iX1 + rectEdge.size().x,
        iY2 = iY1 + rectEdge.size().y;

  if ( edgeType & GUI::GET_RAISED_BORDER )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width == 0 )
    {
      // per Hand zeichnen
      pViewer->SetTexture( 0, NULL );

      pViewer->DrawQuad( iX1, iY1, iX2 - iX1 - 1, 1, GetSysColor( GUI::COL_3DLIGHT ) );
      pViewer->DrawQuad( iX1, iY1, 1, iY2 - iY1 - 1, GetSysColor( GUI::COL_3DLIGHT ) );

      pViewer->DrawQuad( iX1 + 1, iY1 + 1, iX2 - iX1 - 2, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
      pViewer->DrawQuad( iX1 + 1, iY1 + 1, 1, iY2 - iY1 - 2, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );

      pViewer->DrawQuad( iX2 - 2, iY1 + 2, 1, iY2 - iY1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );
      pViewer->DrawQuad( iX1 + 2, iY2 - 2, iX2 - iX1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );

      pViewer->DrawQuad( iX2 - 1, iY1 + 1, 1, iY2 - iY1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
      pViewer->DrawQuad( iX1 + 1, iY2 - 1, iX2 - iX1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
    }
    else
    {
      pViewer->DrawTextureSection( iX1, iY1, m_TextureSection[GUI::BT_EDGE_TOP_LEFT] );
      pViewer->DrawTextureSection( iX1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width, iY1, m_TextureSection[GUI::BT_EDGE_TOP], -1, rectEdge.size().x - m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width - m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].m_Width );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].m_Width, iY1, m_TextureSection[GUI::BT_EDGE_TOP_RIGHT] );
      pViewer->DrawTextureSection( iX1, iY1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Height, m_TextureSection[GUI::BT_EDGE_LEFT], -1, -1, rectEdge.size().y - 2 * m_TextureSection[GUI::BT_EDGE_TOP].m_Height );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_EDGE_RIGHT].m_Width, iY1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Height, m_TextureSection[GUI::BT_EDGE_RIGHT], -1, -1, rectEdge.size().y - 2 * m_TextureSection[GUI::BT_EDGE_TOP].m_Height );
      pViewer->DrawTextureSection( iX1, iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].m_Height, m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT] );
      pViewer->DrawTextureSection( iX1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width, iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM].m_Height, m_TextureSection[GUI::BT_EDGE_BOTTOM], -1, rectEdge.size().x - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].m_Width - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].m_Width );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].m_Width, iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].m_Height, m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT] );
    }
  }
  else if ( edgeType & GUI::GET_SUNKEN_BORDER )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width == 0 )
    {
      // per Hand zeichnen
      pViewer->SetTexture( 0, NULL );

      pViewer->DrawQuad( iX1, iY1, iX2 - iX1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
      pViewer->DrawQuad( iX1, iY1, 1, iY2 - iY1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );

      pViewer->DrawQuad( iX1 + 1, iY1 + 1, iX2 - iX1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );
      pViewer->DrawQuad( iX1 + 1, iY1 + 1, 1, iY2 - iY1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );

      pViewer->DrawQuad( iX2 - 2, iY1 + 2, 1, iY2 - iY1 - 2, GetSysColor( GUI::COL_3DLIGHT ) );
      pViewer->DrawQuad( iX1 + 2, iY2 - 2, iX2 - iX1 - 2, 1, GetSysColor( GUI::COL_3DLIGHT ) );

      pViewer->DrawQuad( iX2 - 1, iY1 + 1, 1, iY2 - iY1 - 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
      pViewer->DrawQuad( iX1 + 1, iY2 - 1, iX2 - iX1 - 1, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
    }
    else
    {
      pViewer->DrawTextureSection( iX1, iY1, m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT] );
      pViewer->DrawTextureSection( iX1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].m_Width, iY1, m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP], -1, rectEdge.size().x - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].m_Width - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].m_Width );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].m_Width, iY1, m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT] );
      pViewer->DrawTextureSection( iX1, iY1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].m_Height, m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT], -1, -1, rectEdge.size().y - 2 * m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].m_Height );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].m_Width, iY1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].m_Height, m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT], -1, -1, rectEdge.size().y - 2 * m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].m_Height );
      pViewer->DrawTextureSection( iX1, iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].m_Height, m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT] );
      pViewer->DrawTextureSection( iX1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].m_Width, iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].m_Height, m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM], -1, rectEdge.size().x - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].m_Width - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].m_Width );
      pViewer->DrawTextureSection( iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].m_Width, iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].m_Height, m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT] );
    }
  }

  /*
  if ( edgeType & GET_RAISED_BORDER_THICK )
  {
    cdPage.Line( iX1, iY1, iX2 - 1, iY1, 0x2994c5 );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, 0x2994c5 );

    cdPage.Line( iX1 + 1, iY1 + 1, iX2 - 2, iY1 + 1, pPage->GetRGB256( 0xffffff ) );
    cdPage.Line( iX1 + 1, iY1 + 1, iX1 + 1, iY2 - 2, pPage->GetRGB256( 0xffffff ) );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, 0 );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, 0 );

    cdPage.Line( iX2 - 1, iY1 + 2, iX2 - 1, iY2 - 1, 0x004a6a );
    cdPage.Line( iX1 + 2, iY2 - 1, iX2 - 1, iY2 - 1, 0x004a6a );
  }
  else if ( edgeType & GET_RAISED_BORDER )
  {
    cdPage.Line( iX1, iY1, iX2 - 1, iY1, 0x2994c5 );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, 0x2994c5 );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, 0x004a6a );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, 0x004a6a );
  }
  else if ( edgeType & GET_SUNKEN_BORDER )
  {
    cdPage.Line( iX1, iY1, iX2 - 1, iY1, 0 );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, 0 );

    cdPage.Line( iX1 + 1, iY1 + 1, iX2 - 2, iY1 + 1, 0x004a6a );
    cdPage.Line( iX1 + 1, iY1 + 1, iX1 + 1, iY2 - 2, 0x004a6a );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, 0x2994c5 );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, 0x2994c5 );
  }
  */
}



void GUIComponent::SetFont( Interface::IFont* pFont )
{
  m_pFont = pFont;
}



void GUIComponent::PrepareStatesForAlpha( CD3DViewer* pViewer, GR::u32 dwColor )
{
  if ( ( dwColor & 0xff000000 ) == 0xff000000 )
  {
    // volles Alpha, braucht kein Alphablending
    pViewer->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  }
  else
  {
    pViewer->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pViewer->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    pViewer->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  }
}



void GUIComponent::DrawMultiTexture( CD3DViewer* pViewer, CDX8MultiTexture* pTexture, int iOffsetX, int iOffsetY )
{
  if ( pTexture == NULL )
  {
    return;
  }

  LPDIRECT3DDEVICE8   pD3DDevice = pViewer->m_pd3dDevice;

  int             vertIndex = 0;


  struct CUSTOMVERTEX
  {
      D3DXVECTOR3 position; // The position
      //float       fRHW;
      D3DCOLOR    color;    // The color
      FLOAT       tu, tv;   // The texture coordinates
  };
  CUSTOMVERTEX vertData[4];

  D3DXVECTOR3   vScale( 1.0f, 1.0f, 0.0f );

  D3DXVECTOR3   vDelta( -0.5f, -0.5f, 0.0f );

  GR::u32       dwColor[4];

  PrepareStatesForAlpha( pViewer, 0xffffffff );

  dwColor[0] = dwColor[1] = dwColor[2] = dwColor[3] = 0xffffffff;

  pViewer->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
  //pD3DDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  for ( int iY = 0; abs( iY * pTexture->m_vSize.y ) < abs( Height() ); ++iY )
  {
    for ( int iX = 0; abs( iX * pTexture->m_vSize.x ) < abs( Width() ); ++iX )
    {
      int     iSectionIndex = 0;

      for ( int iTileY = 0; iTileY < pTexture->m_iTilesY; ++iTileY )
      {
        for ( int iTileX = 0; iTileX < pTexture->m_iTilesX; ++iTileX )
        {
          float     fSectionOffsetX = iX * pTexture->m_vSize.x * vScale.x + iTileX * vScale.x * 256.0f + iOffsetX + vDelta.x,
                    fSectionOffsetY = iY * pTexture->m_vSize.y * vScale.y + iTileY * vScale.y * 256.0f + iOffsetY + vDelta.y;

          tTextureSection*  pSectionInfo = pTexture->m_vectSections[iSectionIndex];

          float     fSizeX = (float)pSectionInfo->m_Width,
                    fSizeY = (float)pSectionInfo->m_Height;
  
          pViewer->SetTexture( 0, pTexture, iSectionIndex );

          vertIndex = 0;

          //vertData[0].fRHW = vertData[1].fRHW = vertData[2].fRHW = vertData[3].fRHW = 0.0f;

          vertData[vertIndex].position.x = fSectionOffsetX;
          vertData[vertIndex].position.y = fSectionOffsetY;
          vertData[vertIndex].position.z = 0.0f;
          vertData[vertIndex].color = dwColor[0];
          vertData[vertIndex].tu = 0.0f;
          vertData[vertIndex].tv = 0.0f;
          ++vertIndex;

          vertData[vertIndex].position.x = fSectionOffsetX + vScale.x * fSizeX;
          vertData[vertIndex].position.y = fSectionOffsetY;
          vertData[vertIndex].position.z = 0.0f;
          vertData[vertIndex].color = dwColor[1];
          vertData[vertIndex].tu = pSectionInfo->m_TU[1];
          vertData[vertIndex].tv = 0.0f;
          ++vertIndex;

          vertData[vertIndex].position.x = fSectionOffsetX;
          vertData[vertIndex].position.y = fSectionOffsetY + vScale.y * fSizeY;
          vertData[vertIndex].position.z = 0.0f;
          vertData[vertIndex].color = dwColor[2];
          vertData[vertIndex].tu = 0.0f;
          vertData[vertIndex].tv = pSectionInfo->m_TV[1];
          ++vertIndex;

          vertData[vertIndex].position.x = fSectionOffsetX + vScale.x * fSizeX;
          vertData[vertIndex].position.y = fSectionOffsetY + vScale.y * fSizeY;
          vertData[vertIndex].position.z = 0.0f;
          vertData[vertIndex].color = dwColor[3];
          vertData[vertIndex].tu = pSectionInfo->m_TU[1];
          vertData[vertIndex].tv = pSectionInfo->m_TV[1];
          ++vertIndex;

          pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

          ++iSectionIndex;
        }
      }
    }
  }

  /*
  pViewer->SetTexture( 0, pTexture );

  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertIndex = 0;
  vertData[vertIndex].position.x = iOffsetX + 0.0f;
  vertData[vertIndex].position.y = iOffsetY + 0.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwColor[0];
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + 0.0f + 100.0f;
  vertData[vertIndex].position.y = iOffsetY + 0.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwColor[1];
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + 0.0f;
  vertData[vertIndex].position.y = iOffsetY + 0.0f + 20.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwColor[2];
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + 0.0f + 100.0f;
  vertData[vertIndex].position.y = iOffsetY + 0.0f + 20.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwColor[3];
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( vertData[0] ) );
  */
}



void GUIComponent::DrawFrame( CD3DViewer* pViewer, CDX8Texture* pTexture, GR::u32 dwFrameColor, int iOffsetX, int iOffsetY,
                               int iX, int iY, int iWidth, int iHeight )
{
  if ( pTexture == NULL )
  {
    return;
  }

  if ( iWidth == 0 )
  {
    iWidth = m_Width;
  }
  if ( iHeight == 0 )
  {
    iHeight = m_Height;
  }

  LPDIRECT3DDEVICE8   pD3DDevice = pViewer->m_pd3dDevice;

  int             vertIndex = 0;

  pViewer->SetTexture( 0, pTexture );

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3 position; // The position
      float       fRHW;
      D3DCOLOR    color;    // The color
      FLOAT       tu, tv;   // The texture coordinates
  };
  CUSTOMVERTEX vertData[4];

  D3DXVECTOR3   vDelta( -0.5f, -0.5f, 0.0f );

  GR::u32         dwColor = ( dwFrameColor & 0x00ffffff ) | ( 0xa0 << 24 );

  PrepareStatesForAlpha( pViewer, dwFrameColor );

  pViewer->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
  pViewer->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].fRHW = vertData[1].fRHW = vertData[2].fRHW = vertData[3].fRHW = 0.1f;

  // LO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX + 4.0f - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX + 4.0f - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // MO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.999f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.999f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.7f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.999f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 3.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.999f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.7f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // LM
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // RO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // RM
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // LU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.0f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // MU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.125f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // RU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 0.875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.875f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 1.0f;
  vertData[vertIndex].tv = 1.0f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  PrepareStatesForAlpha( pViewer, 0x60000000 );
  pViewer->SetTexture( 0, NULL );
  pViewer->DrawQuad( D3DXVECTOR3( iOffsetX + iX + 4.0f, iOffsetY + iY + 4.0f, 1.0f ),
                     D3DXVECTOR3( iWidth - 8.0f, iHeight - 8.0f, 0.0f ),
                     0x60606000 );
}



void GUIComponent::DrawSunkenFrame( CD3DViewer* pViewer, CDX8Texture* pTexture, GR::u32 dwFrameColor,
                                     int iOffsetX, int iOffsetY,
                                     int iX, int iY, int iWidth, int iHeight )
{
  if ( pTexture == NULL )
  {
    return;
  }

  if ( iWidth == 0 )
  {
    iWidth = m_Width;
  }
  if ( iHeight == 0 )
  {
    iHeight = m_Height;
  }

  LPDIRECT3DDEVICE8   pD3DDevice = pViewer->m_pd3dDevice;

  int             vertIndex = 0;

  pViewer->SetTexture( 0, pTexture );

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3 position; // The position
      float       fRHW;
      D3DCOLOR    color;    // The color
      FLOAT       tu, tv;   // The texture coordinates
  };
  CUSTOMVERTEX vertData[4];

  D3DXVECTOR3   vDelta( -0.5f, -0.5f, 0.0f );

  GR::u32         dwColor = ( dwFrameColor & 0x00ffffff ) | ( 0xa0 << 24 );

  PrepareStatesForAlpha( pViewer, dwFrameColor );

  pViewer->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
  pViewer->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].fRHW = vertData[1].fRHW = vertData[2].fRHW = vertData[3].fRHW = 0.1f;

  // LO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX + 4.0f - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX + 4.0f - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // MO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.5f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.5f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // LM
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.5f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3125f;
  vertData[vertIndex].tv = 0.5f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // RO
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.1875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY + 4.0f - 0.5f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.3125f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // RM
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.35f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.35f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.5f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.5f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // LU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.28125f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.1875f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.28125f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );
  
  // MU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3425f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.5f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.3425f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.5f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  // RU
  vertIndex = 0;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight - 4.0f;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.625f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth - 4.0f;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.625f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  vertData[vertIndex].position.x = iOffsetX + iX - 0.5f + iWidth;
  vertData[vertIndex].position.y = iOffsetY + iY - 0.5f + iHeight;
  vertData[vertIndex].position.z = 0.0f;
  vertData[vertIndex].color = dwFrameColor;
  vertData[vertIndex].tu = 0.71875f;
  vertData[vertIndex].tv = 0.71875f;
  ++vertIndex;

  pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( CUSTOMVERTEX ) );

  PrepareStatesForAlpha( pViewer, 0x60000000 );
  pViewer->SetTexture( 0, NULL );
  pViewer->DrawQuad( D3DXVECTOR3( iOffsetX + iX + 4.0f, iOffsetY + iY + 4.0f, 0.0f ),
                     D3DXVECTOR3( iWidth - 8.0f, iHeight - 8.0f, 0.0f ),
                     0x60606000 );
}



void GUIComponent::DisplayText( CD3DViewer* pViewer, 
                                 int iXOffset, int iYOffset,
                                 const char* szText,
                                 GR::u32 textAlignment,
                                 GR::u32 dwColor,
                                 GR::tRect* pRect )
{
  if ( m_pFont == NULL )
  {
    return;
  }

  GR::CDX8ShaderAlphaTest::Apply( *pViewer );

  GR::tPoint    ptText = TextOffset( szText, textAlignment, pRect );
  pViewer->DrawText( (CDX8Font*)m_pFont, 
                     iXOffset + ptText.x, iYOffset + ptText.y,
                     szText,
                     dwColor );
}



void GUIComponent::SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection )
{
  m_TextureSection[eType] = TexSection;

  GUI::tListComponents::iterator   it( m_listComponents.begin() );
  while ( it != m_listComponents.end() )
  {
    GUIComponent*    pChild = (GUIComponent*)*it;

    pChild->SetTextureSection( eType, TexSection );

    ++it;
  }

  RecalcClientRect();
}



void GUIComponent::RecalcClientRect()
{
  m_ClientRect.set( 0, 0, m_Width, m_Height );

  if ( ( m_Edge & GUI::GET_SUNKEN_BORDER )
  ||   ( m_Edge & GUI::GET_RAISED_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width == 0 )
    {
      int   iBorderWidth = 2;
      if ( m_Edge & GUI::GET_FLAT_BORDER )
      {
        iBorderWidth = 1;
      }
      m_ClientRect.inflate( -iBorderWidth, -iBorderWidth );
    }
    else
    {
      m_ClientRect.Left += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width;
      m_ClientRect.Top += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Height;

      m_ClientRect.Right -= m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].m_Width;
      m_ClientRect.Bottom -= m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].m_Height;
    }
  }
}



void GUIComponent::DrawFocusRect( CD3DViewer* pViewer, int iXOffset, int iYOffset, GR::tRect& rc )
{
  pViewer->DrawRect( iXOffset + rc.Left, iYOffset + rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_BACKGROUND ) );
}



int GUIComponent::GetBorderWidth( const GUI::eBorderType eType ) const
{
  if ( ( m_Edge & GUI::GET_SUNKEN_BORDER )
  ||   ( m_Edge & GUI::GET_RAISED_BORDER )
  ||   ( m_Edge & GUI::GET_FLAT_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width == 0 )
    {
      // nichts gesetzt
      if ( eType != GUI::BT_BACKGROUND )
      {
        if ( m_Edge & GUI::GET_FLAT_BORDER )
        {
          return 1;
        }
        return 2;
      }
    }
  }
  return m_TextureSection[eType].m_Width;
}



int GUIComponent::GetBorderHeight( const GUI::eBorderType eType ) const
{
  if ( ( m_Edge & GUI::GET_SUNKEN_BORDER )
  ||   ( m_Edge & GUI::GET_RAISED_BORDER )
  ||   ( m_Edge & GUI::GET_FLAT_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].m_Width == 0 )
    {
      // nichts gesetzt
      if ( eType != GUI::BT_BACKGROUND )
      {
        if ( m_Edge & GUI::GET_FLAT_BORDER )
        {
          return 1;
        }
        return 2;
      }
    }
  }
  return m_TextureSection[eType].m_Height;
}



GR::u32 GUIComponent::GetSysColor( GUI::eColorIndex eColor )
{
  return GUIComponentDisplayer::Instance().GetSysColor( eColor );
}