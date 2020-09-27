#ifndef XVIEWPORT_H
#define XVIEWPORT_H
#pragma once

#include <GR/GRTypes.h>

struct XViewport
{

  GR::i32       X,
                Y,
                Width,
                Height;

  GR::f32       MinZ,
                MaxZ;

  XViewport() :
    X( 0 ),
    Y( 0 ),
    Width( 0 ),
    Height( 0 ),
    MinZ( 0.0f ),
    MaxZ( 1.0f )
  {
  }
};



#endif // XVIEWPORT_H