// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <vector>
#include <set>

#include <GR/XJump3d/LevelMap.h>
#include <GR/XJump3d/GameObject.h>
#include <GR/XJump3d/GameEvent.h>
#include <GR/XJump3d/JumpLua.h>

#include <Spiel/ObjectPath.h>

#include <Grafik/ContextDescriptor.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <DX8/DX8Camera.h>
#include <DX8/TextureSection.h>

#include <Interface/IUpdateable.h>

#include <Interface/IAnimationManager.h>
#include <Interface/IEventListener.h>

#include "XJumpEnums.h"


struct IIOStream;
class CD3DViewer;

class CXJump : public IUpdateAble<float>, public IEventListener<tGameEvent>
{

  public:

    typedef math::vector2t<int>       tPoint;

    typedef std::map<GR::String,CGameObject*>    tMapKnownObjects;

    enum eDirType
    {
      DIR_NONE,
      DIR_UP,
      DIR_DOWN,
      DIR_LEFT,
      DIR_RIGHT,
    };

    enum eActionType
    {
      AT_NONE,
      AT_KILLED,
    };

    struct tTileInfo
    {
      int           iTileSetIndex,
                    iTileX,
                    iTileY,
                    iTileWidth,
                    iTileHeight;
      GR::u32       dwFlags;

      tTileInfo() :
        iTileSetIndex( 0 ),
        iTileX( 0 ),
        iTileY( 0 ),
        iTileWidth( 0 ),
        iTileHeight( 0 ),
        dwFlags( tTextureSection::TSF_DEFAULT )
      {
      }
    };

    typedef std::list<GR::String>              tListMaps;

    typedef std::pair<GR::String,CGameObject*> tPairKnownObject;

    typedef std::vector<tPairKnownObject>       tVectKnownObjects;

    typedef std::vector<GR::String>            tVectInfoTileSets;

    typedef std::vector<tTileInfo>              tVectInfoTiles;

    typedef std::vector<CContextDescriptor>     tVectColissionTiles;

    typedef std::vector<GR::CImage*>            tVectColissionTileSets;

    typedef std::map<GR::String,GR::GAME::CObjectPath>   tMapObjectPaths;

    typedef std::set<WORD>                      tSetTileCategory;

    typedef std::map<eTileCategory,tSetTileCategory>     tMapTileCategories;

    typedef IAnimationManager<int>::tAnimType   tAnimType;
    typedef tAnimType::tAnimationFrame          tAnimFrame;
    typedef std::vector<tAnimationPos>          tVectAnimPositions;
    typedef std::map<GR::u32,GR::u32>               tMapPreferredColissionTile;

    IAnimationManager<int>      m_AnimationManager;

    tVectAnimPositions          m_vectAnimPositions;

    tVectKnownObjects           m_vectKnownObjects;

    tMapKnownObjects            m_mapKnownObjects;

    CLevelMap                   m_currentMap;

    GR::String                 m_strCurrentMapName;

    CGameObject*                m_pControlledObject;

    CLocalRegistry              m_GameVars;

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

    GR::String                 m_strDataPath,
                                m_strOnLoadScript;

    CDX8Camera                  m_Camera;

    CDX8Texture*                m_pTextureBackground;

    CJumpLua                    m_LuaHandler;


    virtual ~CXJump();

    static CXJump&              Instance();


    virtual void                Update( const float fElapsedTime );

    void                        PrepareForStart();

    bool                        BlockedAt( CGameObject* pMovingObject, int iX, int iY, eDirType, eActionType& aType, bool bCanPush = true );
    bool                        StepPossible( CGameObject* pMovingObject, int iX, int iY, int iStepHeight, int& iResultStepHeight, eActionType& atType, eDirType dtDir = DIR_NONE, bool bCanPush = true );
    bool                        IsAreaFree( int iX, int iY, int iWidth, int iHeight );

    math::vector3               Gravity();
    void                        Gravity( math::vector3& vGrav );

    CXJump::tTileInfo*          GetTileInfo( GR::u32 dwTile );
    CContextDescriptor*         GetColissionTileCD( GR::u32 dwTile );
    tTextureSection             GetTileSection( GR::u32 dwTile );

    // Objekt-Database
    int                         AddKnownObject( const GR::String& strName, CGameObject* pObj );
    CGameObject*                GetKnownObject( const GR::String& strName );
    CGameObject*                RenameKnownObject( const GR::String& strOldName, const GR::String& strNewName );

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

    void                        PreloadImages( CD3DViewer& Viewer );

    void                        DisplayTile( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags = 0 );
    void                        DisplayObject( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags );
    void                        DisplayField(  CD3DViewer& Viewer );

    void                        AddSection( CDX8Texture* pTexture, int iX, int iY, int iWidth, int iHeight, GR::u32 dwFlags );

    void                        AddTileToCategory( const eTileCategory eCategory, const GR::u32 dwTile );
    void                        RemoveTileFromCategory( const eTileCategory eCategory, const GR::u32 dwTile );
    bool                        IsTileInCategory( const eTileCategory eCategory, const GR::u32 dwTile ) const;

    CGameObject*                FindCollidingObject( CGameObject* pCollider, const GR::String& strType );
    CGameObject*                GetObjectByID( GR::u32 iID );

    void                        Run();

    void                        ProcessExtraData( CGameObject* pObject );
    void                        StartScriptAsMovie( const GR::String& strScript );


  protected:

    typedef std::vector<tTextureSection>    tVectTiles;

    struct tTempTileDrawInfo
    {
      tTextureSection*      m_pTexSection;
      int                   m_iX,
                            m_iY;
      DWORD                 m_dwAlternativeFlags;
      tTempTileDrawInfo( tTextureSection* pTS, int iX, int iY, DWORD dwAlternativeFlags = 0 ) :
        m_pTexSection( pTS ),
        m_iX( iX ),
        m_iY( iY ),
        m_dwAlternativeFlags( dwAlternativeFlags )
      {
      }
    };

    typedef std::list<tTempTileDrawInfo>    tAlphaTileList;


    tVectTiles                  m_vectTiles;

    tAlphaTileList              m_listAlphaTiles;


    CXJump();

    void                        ClipToCurrentRange( math::vector3& vectPos );
    void                        SoftScrollViewWithObject( CGameObject* pObject, const float fElapsedTime );


    friend class CLevelMap;
    friend class CLevelLayer;

};

#endif // !defined(AFX_XJUMP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
