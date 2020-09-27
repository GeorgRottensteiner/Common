// GameData.cpp: implementation of the CGameData class.
//
//////////////////////////////////////////////////////////////////////

#include "TextureTileSet.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextureTileSet::~CTextureTileSet()
{

  m_vectImages.clear();

}



/*-AddSection-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CTextureTileSet::AddSection( CDX8Texture* pTexture, int iX, int iY, int iWidth, int iHeight )
{

  m_vectImages.push_back( tTextureSection() );

  tTextureSection&    newSect = m_vectImages.back();

  newSect.m_pTexture  = pTexture;
  newSect.m_iXOffset  = iX;
  newSect.m_iYOffset  = iY;
  newSect.m_iWidth    = abs( iWidth );
  newSect.m_iHeight   = abs( iHeight );

  if ( pTexture )
  {
    newSect.m_fTU[0] = ( (float)iX ) / (float)pTexture->m_vSize.x;
    newSect.m_fTV[0] = ( (float)iY ) / (float)pTexture->m_vSize.y;
    newSect.m_fTU[1] = ( (float)iX + iWidth ) / (float)pTexture->m_vSize.x;

    newSect.m_fTV[1] = ( (float)iY + iHeight ) / (float)pTexture->m_vSize.y;
  }

  return (int)m_vectImages.size() - 1;

}



/*-DisplayTile----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CTextureTileSet::DisplayTile( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, DWORD dwColor )
{

  if ( iTile >= m_vectImages.size() )
  {
    return;
  }

  tTextureSection&    Section = m_vectImages[iTile];

  Viewer.SetTexture( 0, Section.m_pTexture );

  if ( dwDisplayFlags & DF_HCENTER )
  {
    fX -= Section.m_iWidth / 2;
  }
  if ( dwDisplayFlags & DF_VCENTER )
  {
    fY -= Section.m_iHeight / 2;
  }

  struct tMyVertex
  {
    D3DXVECTOR3   vectPos;
    float         fRHW;
    DWORD         dwColor;
    float         fTU;
    float         fTV;
  };

  tMyVertex    vertexData[4];

  float   fZ = 0.0f;

  float   fX1 = fX - 0.5f;
  float   fY1 = fY - 0.5f;
  float   fX2 = fX + Section.m_iWidth - 0.5f;
  float   fY2 = fY + Section.m_iHeight - 0.5f;

  int     iTU[2][2],
          iTV[2][2];

  iTU[0][0] = 0;
  iTV[0][0] = 0;
  iTU[1][0] = 1;
  iTV[1][0] = 0;
  iTU[0][1] = 0;
  iTV[0][1] = 1;
  iTU[1][1] = 1;
  iTV[1][1] = 1;

  if ( ( dwDisplayFlags & DF_ROTATE_MASK ) == DF_ROTATE_270 )
  {
    iTU[0][0] = 0;
    iTV[0][0] = 1;
    iTU[1][0] = 0;
    iTV[1][0] = 0;
    iTU[0][1] = 1;
    iTV[0][1] = 1;
    iTU[1][1] = 1;
    iTV[1][1] = 0;
  }
  else if ( ( dwDisplayFlags & DF_ROTATE_MASK ) == DF_ROTATE_180 )
  {
    iTU[0][0] = 1;
    iTV[0][0] = 1;
    iTU[1][0] = 0;
    iTV[1][0] = 1;
    iTU[0][1] = 1;
    iTV[0][1] = 0;
    iTU[1][1] = 0;
    iTV[1][1] = 0;
  }
  else if ( ( dwDisplayFlags & DF_ROTATE_MASK ) == DF_ROTATE_90 )
  {
    iTU[0][0] = 1;
    iTV[0][0] = 0;
    iTU[1][0] = 1;
    iTV[1][0] = 1;
    iTU[0][1] = 0;
    iTV[0][1] = 0;
    iTU[1][1] = 0;
    iTV[1][1] = 1;
  }

  if ( dwDisplayFlags & DF_MIRROR_H )
  {
    int   iTemp = iTU[0][0];
    iTU[0][0] = iTU[1][0];
    iTU[1][0] = iTemp;
    iTemp = iTU[0][1];
    iTU[0][1] = iTU[1][1];
    iTU[1][1] = iTemp;
  }
  if ( dwDisplayFlags & DF_MIRROR_V )
  {
    int   iTemp = iTV[0][0];
    iTV[0][0] = iTV[0][1];
    iTV[0][1] = iTemp;
    iTemp = iTV[1][0];
    iTV[1][0] = iTV[1][1];
    iTV[1][1] = iTemp;
  }

  vertexData[0].fRHW = vertexData[1].fRHW = vertexData[2].fRHW = vertexData[3].fRHW = 0.2f;

  vertexData[0].vectPos = D3DXVECTOR3( fX1, fY1, fZ );
  vertexData[0].dwColor = dwColor;
  vertexData[0].fTU     = Section.m_fTU[iTU[0][0]];
  vertexData[0].fTV     = Section.m_fTV[iTV[0][0]];

  vertexData[1].vectPos = D3DXVECTOR3( fX2, fY1, fZ );
  vertexData[1].dwColor = dwColor;
  vertexData[1].fTU     = Section.m_fTU[iTU[1][0]];
  vertexData[1].fTV     = Section.m_fTV[iTV[1][0]];

  vertexData[2].vectPos = D3DXVECTOR3( fX1, fY2, fZ );
  vertexData[2].dwColor = dwColor;
  vertexData[2].fTU     = Section.m_fTU[iTU[0][1]];
  vertexData[2].fTV     = Section.m_fTV[iTV[0][1]];

  vertexData[3].vectPos = D3DXVECTOR3( fX2, fY2, fZ );
  vertexData[3].dwColor = dwColor;
  vertexData[3].fTU     = Section.m_fTU[iTU[1][1]];
  vertexData[3].fTV     = Section.m_fTV[iTV[1][1]];

  Viewer.SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  Viewer.m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertexData, sizeof( vertexData[0] ) );

}




int CTextureTileSet::Width( size_t iTile ) const
{

  if ( iTile >= m_vectImages.size() )
  {
    return 0;
  }
  return m_vectImages[iTile].m_iWidth;

}



int CTextureTileSet::Height( size_t iTile ) const
{

  if ( iTile >= m_vectImages.size() )
  {
    return 0;
  }
  return m_vectImages[iTile].m_iHeight;

}



size_t CTextureTileSet::TileCount() const
{

  return m_vectImages.size();

}



void CTextureTileSet::AddIndex( const GR::String& strDesc, size_t iIndex )
{

  m_mapIndices[strDesc] = iIndex;

}



size_t CTextureTileSet::Index( const GR::String& strDesc )
{

  tMapIndices::iterator   it( m_mapIndices.find( strDesc ) );
  if ( it == m_mapIndices.end() )
  {
    return 0;
  }
  return it->second;

}