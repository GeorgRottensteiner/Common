#pragma once


#include <string>
#include <GR/GRTypes.h>



namespace Xtreme
{
  namespace SupportedDisplayConfiguration
  {
    enum Value
    {
      NONE                  = 0,
      LANDSCAPE             = 0x00000001,
      LANDSCAPE_FLIPPED     = 0x00000002,
      PORTRAIT              = 0x00000004,
      PORTRAIT_FLIPPED      = 0x00000008,

      ALL = LANDSCAPE | LANDSCAPE_FLIPPED | PORTRAIT | PORTRAIT_FLIPPED
    };
  }

  namespace StartupFlags
  {
    enum Value
    {
      NONE = 0,
      FULL_SCREEN         = 0x00000001,
      DESKTOP_FULL_SCREEN = 0x00000002      // size window to desktop, no hardware resolution change
    };
  }

  struct EnvironmentConfig
  {
    int           StartUpWidth;
    int           StartUpHeight;
    GR::String    Caption;
    // allow window to be resized?
    bool          FixedSize;
    // means introduce black bars left/right and/or top/bottom to keep ratio
    bool          KeepAspectRatio;
    GR::u32       IconResourceID;
    GR::u32       SupportedDisplayConfigurations;
    // factor to fixed scale of pixel (e.g. double pixel size)
    GR::tFPoint   FixedVirtualSizeFactor;

    GR::u32       StartupFlags;


    EnvironmentConfig() :
      StartUpWidth( 800 ),
      StartUpHeight( 600 ),
      Caption( "Sample Application" ),
      FixedSize( false ),
      KeepAspectRatio( false ),
      IconResourceID( 0 ),
      SupportedDisplayConfigurations( SupportedDisplayConfiguration::ALL ),
      StartupFlags( StartupFlags::NONE )
    {
    }
  };
}
