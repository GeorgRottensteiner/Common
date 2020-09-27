// GameData.h: interface for the CGameData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTURE_TILE_H__888F6998_ED45_429B_99E5_B58244D333F8__INCLUDED_)
#define AFX_TEXTURE_TILE_H__888F6998_ED45_429B_99E5_B58244D333F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DX8/DX8Viewer.h"

#include "TextureSection.h"

#include <vector>


class CTextureTileSet
{

  public:

    enum eDisplayFlags
    {
      DF_DEFAULT = 0,
      DF_ROTATE_90      = 0x00000001,
      DF_ROTATE_180     = 0x00000002,
      DF_ROTATE_270     = 0x00000003,
      DF_ROTATE_MASK    = 0x00000003,
      DF_HCENTER        = 0x00000004,
      DF_VCENTER        = 0x00000008,
      DF_MIRROR_H       = 0x00000010,
      DF_MIRROR_V       = 0x00000020,
    };


    typedef std::vector<tTextureSection>    tVectImages;

    typedef std::map<GR::String,size_t>    tMapIndices;

    tVectImages                             m_vectImages;

    tMapIndices                             m_mapIndices;


	  virtual ~CTextureTileSet();


    int                       AddSection( CDX8Texture* pTexture, int iX, int iY, int iWidth, int iHeight );
    void                      DisplayTile( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, DWORD dwColor = 0xffffffff );

    int                       Width( size_t iTile ) const;
    int                       Height( size_t iTile ) const;

    size_t                    TileCount() const;

    void                      AddIndex( const GR::String& strDesc, size_t iIndex );
    size_t                    Index( const GR::String& strDesc );

};

#endif // !defined(AFX_TEXTURE_TILE_H__888F6998_ED45_429B_99E5_B58244D333F8__INCLUDED_)
