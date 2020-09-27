#ifndef XLIGHT_H
#define XLIGHT_H
#pragma once

#include <GR/GRTypes.h>

struct XLight
{
  enum eLightType
  {
    LT_INVALID = 0,
    LT_POINT,
    LT_SPOT,
    LT_DIRECTIONAL,
  };

  eLightType      m_Type;

  GR::u32         m_Diffuse;
  GR::u32         m_Specular;
  GR::u32         m_Ambient;

  GR::tVector     m_Position;
  GR::tVector     m_Direction;

  GR::f32         m_Range,
                  m_Falloff,
                  m_Attenuation0,
                  m_Attenuation1,
                  m_Attenuation2,
                  m_Theta,
                  m_Phi;

  XLight() :
    m_Type( LT_INVALID ),
    m_Diffuse( 0 ),
    m_Specular( 0 ),
    m_Ambient( 0 ),
    m_Range( 0.0f ),
    m_Falloff( 0.0f ),
    m_Attenuation0( 0.0f ),
    m_Attenuation1( 0.0f ),
    m_Attenuation2( 0.0f ),
    m_Theta( 0.0f ),
    m_Phi( 0.0f )
  {
  }
};



#endif // XLIGHT_H