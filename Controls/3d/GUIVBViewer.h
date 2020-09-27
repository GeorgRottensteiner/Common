#pragma once

#include <Controls/3d/GUIScreen.h>

#include <DX8/VertexBuffer.h>
#include <DX8/Orientation.h>
#include <DX8/DX8RenderStateBlock.h>



class CGUIVBViewer : public CGUIScreen, public CDX8RenderStateBlock
{

  protected:

    CVertexBuffer*            m_pVB;
    CDX8Texture*              m_pTexture;

    COrientation              m_Orientation;

    D3DXMATRIX                m_matWorld;

    math::vector3             m_vectDeltaRotate,
                              m_vectCurrentRotation;


  public:

    CGUIVBViewer( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId = 0 );

    void                      SetVertexBuffer( CVertexBuffer* pVB = NULL, CDX8Texture* pTexture = NULL );
    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    COrientation&             Orientation();

    void                      SetTransform( const D3DXMATRIX& matWorld );

    virtual void              Update( float fElapsedTime );

};
