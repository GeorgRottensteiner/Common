#ifndef XMATERIAL_H
#define XMATERIAL_H
#pragma once

#include <GR/GRTypes.h>

struct XMaterial
{
  GR::u32       Diffuse,
                Ambient,
                Specular,
                Emissive;

  GR::f32       Power;

  XMaterial() :
    Diffuse( 0 ),
    Ambient( 0xffffffff ),
    Specular( 0 ),
    Emissive( 0 ),
    Power( 0.0f )
  {
  }
};



#endif // XMATERIAL_H