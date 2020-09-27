// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_PROJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable:4786 )
#include <string>
#include <map>
#include <vector>

#include <GR\JumpNRun\LevelMap.h>
#include <GR\JumpNRun\AnimatedTileSet.h>

#include <GR\LocalRegistry\LocalRegistry.h>



class CJRProject : CLocalRegistry
{
  public:

    GR::String               m_strProjectPath;

    CJRProject();
    virtual ~CJRProject();

    tMapAnimatedTileSets      m_mapAnimatedTileSets;

    tListMaps                 m_listMaps;

    CLevelMap*                m_pCurrentMap;


    bool                      Save( const char* szFileName );
    bool                      Load( const char* szFileName );


    bool                      SetCurrentMap( const char* szName );

    bool                      Display( CGfxPage *pPage, DWORD dwElapsedTicks );

    bool                      BlockedAt( int iX, int iY );

    bool                      ScrollTo( float fX, float fY );


};

#endif // !defined(AFX_PROJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
