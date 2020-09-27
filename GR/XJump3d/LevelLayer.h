// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELLAYER_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_LEVELLAYER_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <list>

#include <Spiel/2dLayer.h>

#include <Interface/IAnimationManager.h>
#include <Interface/IIOStream.h>
#include <Interface/IUpdateable.h>

#include <math/vector2t.h>
#include <math/vector3.h>
#include <math/tRect.h>

#include "XJumpEnums.h"


class CGameObject;

class CLevelLayer : public C2DLayer<WORD>, public IUpdateAble<float>
{

  public:

    typedef std::list<CGameObject*>             tListObjects;


  protected:

    tListObjects            m_listAliveObjects;


  public:


    typedef std::pair<int,int>                  tPairCoord;

    typedef std::map<tPairCoord,tListObjects>   tMapSleepingObjects;

    typedef math::vector2t<int>                 tPoint;


    typedef std::list<tTriggerArea>             tListTrigger;


    enum    eLayerType
    {
      LT_INVALID      = 0,
      LT_BLOCKS       = 1,
      LT_COLISSION    = 2,
      LT_FLAGS        = 3,
    };

    enum eLayerFlags
    {
      LF_DEFAULT      = 0,
      LF_MAIN_LAYER   = 0x00000001,
      LF_VISIBLE      = 0x00000002,
    };

    eLayerType              m_Type;

    DWORD_PTR               m_dwUserData;

    DWORD                   m_dwFlags;

    math::vector3           m_vRelativScrollSpeed;

    math::vector2t<int>     m_vTileSize;

    tMapSleepingObjects     m_mapSleepingObjects;

    math::tRect<int>        m_rectVisualRange;

    tListTrigger            m_listTrigger;


    CLevelLayer();
    virtual ~CLevelLayer();


    virtual bool            Create( int iWidth, int iHeight, const char* szDesc, eLayerType eType = LT_BLOCKS );

    void                    Save( IIOStream& File );
    void                    Load( IIOStream& File );

    void                    AddSleepingObject( CGameObject* pObj );
    void                    RemoveSleepingObject( CGameObject* pObj );

    void                    AddObject( CGameObject* pObj );
    CGameObject*            GetObject( DWORD dwType );
    CGameObject*            GetObject( const GR::String& strType );
    CGameObject*            GetObjectByID( GR::u32 dwID );

    void                    Update( const float fElapsedTime );

    void                    SetVisualRange( int iX = -1, int iY = -1, int iWidth = -1, int iHeight = -1 );

    void                    PrepareForStart();

    void                    SpawnObject( CGameObject* pObj );
    void                    SpawnObjects( float fX, float fY, float fWidth, float fHeight );
    void                    SleepObject( CGameObject* pObj );

    CGameObject*            FindCollidingObject( CGameObject* pCollider, const GR::String& strType );

    friend class CXJump;

};


#endif // !defined(AFX_LEVELLAYER_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
