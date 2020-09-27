#if !defined(AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <vector>
#include <set>

#include <GR/XJumpX/LevelMap.h>
#include <GR/XJumpX/GameObject.h>
#include <GR/XJumpX/GameEvent.h>
#include <GR/XJumpX/JumpLua.h>

#include <Game/MovementPath.h>

#include <Grafik/ContextDescriptor.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <Xtreme/XCamera.h>
#include <Xtreme/XTextureSection.h>

#include <Interface/IUpdateable.h>

#include <Interface/IAnimationManager.h>
#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <Misc/CloneFactory.h>

#include "XJumpEnums.h"

struct IIOStream;
class XRenderer;

class CXJump : public IUpdateable<float>, 
               public EventListener<tGameEvent>,
               public EventProducer<tGameEvent>
{

  public:

    typedef math::vector2<int>       tPoint;

    typedef std::map<GR::String,CGameObject*>    tMapKnownObjects;

    typedef std::list<GR::String>              tListMaps;

    typedef std::pair<GR::String,CGameObject*> tPairKnownObject;

    typedef std::vector<tPairKnownObject>       tVectKnownObjects;

    typedef std::vector<GR::String>             tVectInfoTileSets;

    typedef std::vector<XJump::tTileInfo>       tVectInfoTiles;

    typedef std::vector<GR::Graphic::ContextDescriptor>     tVectColissionTiles;

    typedef std::vector<GR::Graphic::Image*>            tVectColissionTileSets;

    typedef std::map<GR::String,GR::Game::Path::MovementPath>   tMapObjectPaths;

    typedef std::set<WORD>                      tSetTileCategory;

    typedef std::map<XJump::eTileCategory,tSetTileCategory>     tMapTileCategories;

    typedef IAnimationManager<int>::tAnimType   tAnimType;
    typedef tAnimType::tAnimationFrame          tAnimFrame;
    typedef std::vector<tAnimationPos>          tVectAnimPositions;
    typedef std::map<GR::u32,GR::u32>               tMapPreferredColissionTile;
    typedef std::map<GR::String,GR::u32>           tMapAnimations;

    IAnimationManager<int>      m_AnimationManager;

    tMapAnimations              m_mapAnimations;

    tVectAnimPositions          m_vectAnimPositions;

    tVectKnownObjects           m_vectKnownObjects;

    tMapKnownObjects            m_mapKnownObjects;

    CLevelMap                   m_currentMap;

    GR::String                 m_strCurrentMapName;

    CGameObject*                m_pControlledObject;

    LocalRegistry               m_GameVars;

    CloneFactory                m_CloneFactory;

    // Spielparameter
    math::vector3               m_vGravity;

    float                       m_fFallAsleepRange;

    tVectColissionTiles         m_vectColissionTiles;

    tVectColissionTileSets      m_vectColissionTileSets;

    tVectInfoTileSets           m_vectInfoTileSets,
                                m_vectInfoColissionTileSets;

    tVectInfoTiles              m_vectInfoTiles,
                                m_vectInfoColissionTiles;

    tMapPreferredColissionTile  m_mapPreferredColissionTile;

    tMapObjectPaths             m_mapObjectPaths;

    tMapTileCategories          m_mapTileCategories;

    tListMaps                   m_listMaps;

    bool                        m_bFrozen,
                                m_bMovieMode;

    tPoint                      m_ptViewportExtents;

    GR::String                  m_strDataPath,
                                m_strOnLoadScript,
                                m_strOnGameStartScript;

    XCamera                     m_Camera;

    XTexture*                   m_pTextureBackground;

    CJumpLua                    m_LuaHandler;

    GR::u32                     Animation( const GR::String& strAnimID ) const;


    virtual ~CXJump();

    static CXJump&              Instance();


    virtual void                Update( const float fElapsedTime );

    void                        PrepareForStart();

    bool                        BlockedAt( CGameObject* pMovingObject, int iX, int iY, XJump::eDirType, XJump::eActionType& aType, bool bCanPush = true );
    bool                        StepPossible( CGameObject* pMovingObject, int iX, int iY, int iStepHeight, int& iResultStepHeight, XJump::eActionType& atType, XJump::eDirType dtDir = XJump::DIR_NONE, bool bCanPush = true );
    bool                        IsAreaFree( int iX, int iY, int iWidth, int iHeight );

    math::vector3               Gravity();
    void                        Gravity( math::vector3& vGrav );

    XJump::tTileInfo*           GetTileInfo( GR::u32 dwTile );
    GR::Graphic::ContextDescriptor*         GetColissionTileCD( GR::u32 dwTile );
    XTextureSection             GetTileSection( GR::u32 dwTile );

    // Objekt-Database
    int                         AddKnownObject( const GR::String& strName, CGameObject* pObj );
    CGameObject*                GetKnownObject( const GR::String& strName );
    CGameObject*                RenameKnownObject( const GR::String& strOldName, const GR::String& strNewName );
    void                        RemoveKnownObject( const GR::String& Name );

    void                        RemoveMap( const GR::String& MapName );

    GR::String                 GetObjectType( CGameObject* pObj );

    void                        RegisterObject( const GR::String& strName, CGameObject* pObj, bool bDontKeepValues = false );
    CGameObject*                CreateObject( const GR::String& strName );
    bool                        IsObjectType( CGameObject* pObject, const GR::String& strName );

    void                        Freeze( bool bFreeze );
    void                        MovieMode( bool bMovie );

    void                        SetViewport( int iWidth, int iHeight );


    void                        Clear();

    bool                        Load( const char* szFileName );
    void                        Load( IIOStream& File );

    bool                        Save( const char* szFileName );
    void                        Save( IIOStream& File );

    void                        SaveGameState( IIOStream& Stream );
    void                        LoadGameState( IIOStream& Stream );

    bool                        InitInstance();

    bool                        ProcessEvent( const tGameEvent& Event );

    bool                        OnTopOfPlatform( CGameObject* pObject );

    void                        PreloadImages( XRenderer& Viewer );

    void                        DisplayTile( XRenderer& Viewer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags = -1 );
    void                        DisplayTileDirect( XRenderer& Viewer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags = -1 );
    void                        DisplayObject( XRenderer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, GR::u32 dwColor = 0xffffffff, int iWidth = -1, int iHeight = -1 );
    void                        DisplayObjectMasked( XRenderer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, GR::u32 dwColor = 0xffffffff, int iWidth = -1, int iHeight = -1 );
    void                        DisplayField(  XRenderer& Viewer );

    void                        AddSection( XTexture* pTexture, int iX, int iY, int iWidth, int iHeight, GR::u32 dwFlags );

    void                        AddTileToCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile );
    void                        RemoveTileFromCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile );
    bool                        IsTileInCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile ) const;

    CGameObject*                FindCollidingObject( CGameObject* pCollider, const GR::String& strType = GR::String() );
    CGameObject*                GetObjectByID( GR::u32 iID );
    CGameObject*                GetObjectByType( const GR::String& strType );

    GR::tPoint                  ObjectLocationOnScreen( CGameObject* pObj );

    void                        EnableTrigger( GR::u32 dwTriggerID, bool bEnable );

    void                        StartGame();
    void                        StartLoadedGame();

    void                        ProcessExtraData( CGameObject* pObject );
    void                        StartScriptAsMovie( const GR::String& strScript );


  public:

    typedef std::vector<XTextureSection>    tVectTiles;

    tVectTiles                  m_vectTiles;


  protected:

    struct tTempTileDrawInfo
    {
      XTextureSection*      m_pTexSection;
      int                   m_iX,
                            m_iY;
      DWORD                 m_dwAlternativeFlags;
      tTempTileDrawInfo( XTextureSection* pTS, int iX, int iY, DWORD dwAlternativeFlags = 0 ) :
        m_pTexSection( pTS ),
        m_iX( iX ),
        m_iY( iY ),
        m_dwAlternativeFlags( dwAlternativeFlags )
      {
      }
    };

    typedef std::list<tTempTileDrawInfo>    tAlphaTileList;


    

    tAlphaTileList              m_listAlphaTiles;


    CXJump();

    void                        ClipToCurrentRange( math::vector3& vectPos );
    void                        SoftScrollViewWithObject( CGameObject* pObject, const float fElapsedTime );


    friend class CLevelMap;
    friend class CLevelLayer;

};

#endif // !defined(AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
