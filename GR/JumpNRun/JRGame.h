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
#include <GR\JumpNRun\TileSet.h>
#include <GR\JumpNRun\AnimationList.h>
#include <GR\JumpNRun\JRObject.h>

#include <GR\LocalRegistry\LocalRegistry.h>


typedef std::map<DWORD,DWORD>   tMapCollisionTypes;

class CJRGame : public CLocalRegistry
{
  public:

    GR::String               m_strProjectPath;

    CJRGame();
    virtual ~CJRGame();

    tMapTileSets              m_mapTileSets;

    CAnimationList            m_AnimationList;

    tListMaps                 m_listMaps;

    tMapJRObjects             m_knownObjects;

    CLevelMap*                m_pCurrentMap;

    tMapCollisionTypes        m_mapCollisionTypes,
                              m_mapCollisionTypesReverse;

    bool                      Save( const char* szFileName );
    bool                      Load( const char* szFileName );


    bool                      SetCurrentMap( const char* szName );
    bool                      SetMainLayer( const char* szName );

    bool                      Display( CGfxPage *pPage, DWORD dwElapsedTicks );

    bool                      BlockedAt( int iX, int iY );

    bool                      ScrollTo( float fX, float fY );

    // Kollisionsmaske
    void                      CollisionType( DWORD dwUserCollisionType, DWORD dwColorValue );
    DWORD                     GetCollisionTypeAt( int iX, int iY );
    DWORD                     CollideValueAt( int iX, int iY );


};

#endif // !defined(AFX_PROJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
