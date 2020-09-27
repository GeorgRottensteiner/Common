#pragma once

#include <Controls/Xtreme/GUIScreen.h>

#include <Xtreme/XVertexBuffer.h>
#include <Xtreme/XOrientation.h>



class GUIVBViewer : public GUIScreen
{

  protected:

    XVertexBuffer*            m_pVB;
    XTexture*                 m_pTexture;

    XOrientation              m_Orientation;

    math::matrix4             m_matWorld;

    GR::tVector               m_vectDeltaRotate,
                              m_vectCurrentRotation;


  public:

    DECLARE_CLONEABLE( GUIVBViewer, "VBViewer" )

    GUIVBViewer( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0 );

    void                      SetVertexBuffer( XVertexBuffer* pVB = NULL, XTexture* pTexture = NULL );
    virtual void              DisplayOnPage( XRenderer* pRenderer, int iOffsetX = 0, int iOffsetY = 0 );

    XOrientation&             Orientation();

    void                      SetTransform( const math::matrix4& matWorld );

    virtual void              Update( float fElapsedTime );

};
