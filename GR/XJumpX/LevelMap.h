#if !defined(AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <string>
#include <list>
#include <vector>

#include <Game/2dLayer.h>

#include <Interface/IAnimationManager.h>
#include <Interface/IUpdateable.h>

#include <GR/GRTypes.h>

#include <GR/XJumpX/LevelLayer.h>
#include <GR/XJumpX/ExtraData.h>



class CXJump;

class CLevelMap : public IUpdateable<float>
{
  public:

    enum MapFlags
    {
      MF_OPEN_TOP             = 0x00000001,
      MF_OPEN_BOTTOM          = 0x00000002,
      MF_OPEN_LEFT            = 0x00000004,
      MF_OPEN_RIGHT           = 0x00000008,
    };

    typedef std::list<CLevelLayer>              tListLayer;

    typedef std::map<GR::u32,tExtraData>          tMapExtraData;

    GR::String           m_strDescription,
                          m_strFileName;

    tListLayer            m_listLayers;

    tMapExtraData         m_mapExtraData;

    GR::up                m_dwUserData;

    GR::u32               m_Flags;

    math::vector3         m_vectScrollOffset;

    CLevelLayer*          m_pMainLayer;

    GR::String           m_Script;

    

    CLevelMap();
    virtual ~CLevelMap();


    void                        Clear();

    virtual bool                Load( const char* szFileName );
    virtual bool                Save( const char* szFileName );

    void                        Load( IIOStream& File );
    void                        Save( IIOStream& File );

    CLevelLayer&                AddLayer();

    CLevelLayer*                GetLayer( const GR::String& strLayerName );
    CLevelLayer*                GetCollisionLayer();

    int                         CollisionTypeAt( int iX, int iY );

    virtual void                Update( const float fElapsedTime );
    void                        PrepareForStart();

    // Extra-Data
    GR::u32                     AddExtraData();
    void                        RemoveExtraData( GR::u32 dwEntry );
    tExtraData*                 GetExtraData( GR::u32 dwEntry );

    CGameObject*                FindCollidingObject( CGameObject* pCollider, const GR::String& strType );

    void                        CenterOnObject( CGameObject* pObject );

    void                        RemoveAllObjectsOfType( const GR::String& Type );
  
};

typedef std::list<CLevelMap>   tListMaps;

#endif // !defined(AFX_LEVELMAP_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
