#ifndef __IDX8_SHADER_H__
#define __IDX8_SHADER_H__
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
#include <d3dx8.h>
#include <DX8/DX8Viewer.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

namespace GR
{

enum eShaderType
{
  ST_NONE = 0,
  ST_FLAT,
  ST_ALPHA_TEST,
  ST_ALPHA_BLEND,
  ST_ALPHA_BLEND_AND_TEST,
  ST_ADDITIVE,
  ST_50_PERCENT_BLEND,
};


struct IDX8Shader
{

  eShaderType       m_Type;


  IDX8Shader() :
    m_Type( ST_NONE )
  {
  }

  virtual ~IDX8Shader()
  {
  }

  static void Apply( CD3DViewer& Viewer )
  {
  }

};


}; // namespace GR

#endif // __IDX8_SHADER_H__