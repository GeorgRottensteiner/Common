#ifndef XDISPLAY_MODE_H
#define XDISPLAY_MODE_H

#include <Grafik/ImageData.h>



struct XRendererDisplayMode
{
  GR::u32                       Width,
                                Height;
  GR::Graphic::eImageFormat     ImageFormat;
  bool                          FullScreen;



  XRendererDisplayMode( GR::u32 Width = 0, GR::u32 Height = 0, GR::Graphic::eImageFormat imgFormat = GR::Graphic::IF_UNKNOWN, bool FullScreen = false ) :
    Width( Width ),
    Height( Height ),
    ImageFormat( imgFormat ),
    FullScreen( FullScreen )
  {
  }



  bool FormatIsEqualTo( const XRendererDisplayMode& rhsDisplayMode ) const
  {
    if ( ( Width != rhsDisplayMode.Width )
    ||   ( Height != rhsDisplayMode.Height )
    ||   ( ImageFormat != rhsDisplayMode.ImageFormat ) )
    {
      return false;
    }
    return true;
  }



  bool operator==( const XRendererDisplayMode& rhsDisplayMode ) const
  {
    if ( ( Width != rhsDisplayMode.Width )
    ||   ( Height != rhsDisplayMode.Height )
    ||   ( ImageFormat != rhsDisplayMode.ImageFormat )
    ||   ( FullScreen != rhsDisplayMode.FullScreen ) )
    {
      return false;
    }
    return true;
  }

};



#endif // XDISPLAY_MODE_H