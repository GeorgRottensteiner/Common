#ifndef __DX8_SHADER_H__
#define __DX8_SHADER_H__
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <windows.h>
#include <DX8/IDX8Shader.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

namespace GR
{

struct CDX8ShaderFlat : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    Viewer.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    Viewer.SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  }

};

struct CDX8ShaderAlphaBlend : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    Viewer.SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    Viewer.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    Viewer.SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  }

};

struct CDX8Shader50PercentBlend : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    Viewer.SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
    Viewer.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    Viewer.SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  }

};

struct CDX8ShaderAlphaTest : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    Viewer.SetRenderState( D3DRS_ALPHAREF, 8 );
    Viewer.SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }

};


struct CDX8ShaderAlphaBlendAndTest : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    Viewer.SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    Viewer.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    Viewer.SetRenderState( D3DRS_ALPHAREF, 8 );
    Viewer.SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    Viewer.SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  }

};


struct CDX8ShaderAdditive : public IDX8Shader
{

  static void Apply( CD3DViewer& Viewer )
  {
    Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    Viewer.SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    Viewer.SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    Viewer.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    Viewer.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    Viewer.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    Viewer.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    Viewer.SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
  }

};


}; // namespace GR

#endif // __DX8_SHADER_H__