// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#pragma warning( disable:4786 )
#include <string>
#include <map>
#include <vector>

#include <Spiel\2dLayer.h>
#include <GR\JumpNRun\TileSet.h>
#include <GR\JumpNRun\AnimationList.h>
#include <GR\JumpNRun\JRObject.h>

enum LayerType
{
  LT_INVALID = 0,
  LT_BLOCKS,
  LT_FLAGS,
  LT_COLLISION,
  LT_OBJECTS,
};



class CLevelLayer : public C2dLayer<WORD>
{

  public:

    CAnimationList*     m_pAnimationList;

    WORD                m_wType;

    DWORD               m_dwUserData;

    int                 m_iRelativScrollSpeedX,
                        m_iRelativScrollSpeedY,
                        m_iTileWidth,
                        m_iTileHeight;

    CLevelLayer();
    virtual ~CLevelLayer();

    virtual bool            Create( int iWidth, int iHeight, const char* szDesc );

};

typedef std::list<CLevelLayer>           tListLayer;

class CLevelMap
{
  public:

    GR::String           m_strDescription,
                          m_strFileName;

    tListLayer            m_listLayers;

    tMapJRObjectList      m_mapObjects;

    DWORD                 m_dwUserData;

    int                   m_iScrollX,
                          m_iScrollY;

    CLevelLayer*          m_pMainLayer;


    CLevelMap();
    virtual ~CLevelMap();


    virtual bool    Load( const char* szFileName );
    virtual bool    Save( const char* szFileName );

};

typedef std::list<CLevelMap>   tListMaps;

#endif // !defined(AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
