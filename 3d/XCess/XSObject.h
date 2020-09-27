#ifndef XSOBJECT_H
#define XSOBJECT_H


#include <DX8/IShadedRenderAble.h>

#include <DX8/Frustum.h>

#include <DX8/Orientation.h>

#include <3d/XCess/XSID.h>


class CSceneNode;
class CDX8Texture;
class CRenderManager;

class CXSObject : public IShadedRenderAble, public COrientation
{

  public:

    enum eXSFlags
    {
      XS_PICKABLE = 0x00000001,
      XS_DEFAULT  = 0,
    };


  protected:

    CBoundingBox                        m_BoundingBox;

    CDX8Texture*                        m_pTexture;

    CXSID                               m_ID;

    GR::u32                             m_Flags;

    GR::up                              m_dwItemData;


  public:


    CSceneNode*                         m_pNode;


    CXSObject( GR::u32 xsFlags = XS_DEFAULT );
    virtual ~CXSObject();

    void                                Texture( CDX8Texture* pTexture );
    CBoundingBox                        BoundingBox();
    CBoundingBox                        TransformedBoundingBox();
    math::matrix4                       Transformation();

    virtual void                        Update( const float fElapsedTime );

    void                                ID( const CXSID& ID );
    const CXSID&                        ID() const;

    void                                ItemData( const GR::up dwItemData );
    GR::up                              ItemData() const;

    void                                ModifyFlags( const GR::u32 dwAdd, const GR::u32 dwRemove );
    GR::u32                             Flags() const;

    virtual void                        Render( CD3DViewer& Viewer );

    virtual void                        ToQueue( CRenderManager& RManager, CFrustum& Frustum );

};

#endif // XSOBJECT_H

