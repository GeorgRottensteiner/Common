// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_TILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <map>
#include <vector>

namespace GR
{
  class CImage;
};

typedef std::vector<GR::CImage*>   tVectImages;



class CTileSet
{
  public:

    tVectImages               m_vectImages;

    GR::String               m_strDescription,
                              m_strSetFileName;

    int                       m_iTileWidth,
                              m_iTileHeight;

    DWORD                     m_dwTransparentColor;

    bool                      m_bTransparent;

    CTileSet();
    virtual ~CTileSet();


    bool                      ReloadImages( void );

};

typedef std::map<GR::String,CTileSet>    tMapTileSets;

#endif // !defined(AFX_TILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
