#ifndef XSOBJECT_H
#define XSOBJECT_H


#include <Xtreme/IShadedRenderAble.h>

#include <Xtreme/XFrustum.h>

#include <Xtreme/XOrientation.h>

#include "XSID.h"


class CSceneNode;
class XTexture;
class CRenderManager;

class CXSObject : public IShadedRenderAble, public XOrientation
{

  public:

    enum eXSFlags
    {
      XS_PICKABLE = 0x00000001,
      XS_DEFAULT  = 0,
    };


  protected:

    XBoundingBox                        m_BoundingBox;

    XTexture*                           m_pTexture;

    CXSID                               m_ID;

    GR::u32                             m_Flags;

    GR::up                              m_dwItemData;


  public:


    CSceneNode*                         m_pNode;


    CXSObject( GR::u32 xsFlags = XS_DEFAULT );
    virtual ~CXSObject();

    void                                Texture( XTexture* pTexture );
    XBoundingBox                        BoundingBox();
    XBoundingBox                        TransformedBoundingBox();
    math::matrix4                       Transformation();

    virtual void                        Update( const float fElapsedTime );

    void                                ID( const CXSID& ID );
    const CXSID&                        ID() const;

    void                                ItemData( const GR::up dwItemData );
    GR::up                              ItemData() const;

    void                                ModifyFlags( const GR::u32 dwAdd, const GR::u32 dwRemove );
    GR::u32                             Flags() const;

    virtual void                        Render( XRenderer& Renderer );

    virtual void                        ToQueue( CRenderManager& RManager, XFrustum& Frustum );

};

#endif // XSOBJECT_H

