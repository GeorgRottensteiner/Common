#ifndef IMAGESET_H
#define IMAGESET_H


#include <vector>
#include <Grafik/ImageData.h>

#include <Lang/GlobalAllocator.h>



struct tImageSetLayer
{
  GR::Graphic::ImageData*     pImageData;
  GR::Graphic::ImageData*     pImageDataMask;



  tImageSetLayer() :
    pImageData( NULL ),
    pImageDataMask( NULL )
  {
  }

  ~tImageSetLayer()
  {
    delete pImageData;
    pImageData = NULL;
    delete pImageDataMask;
    pImageDataMask = NULL;
  }
};



struct tImageSetFrame
{
  typedef std::vector<tImageSetLayer*,GR::STL::global_allocator<tImageSetLayer*> >    tLayers;

  tLayers       Layers;
  GR::u32       DelayMS;    // Millisekunden
  int           XOffset,
                YOffset,
                HotSpotX,
                HotSpotY;

  tImageSetFrame() :
    DelayMS( 0 ),
    XOffset( 0 ),
    YOffset( 0 ),
    HotSpotX( 0 ),
    HotSpotY( 0 )
  {
  }

  ~tImageSetFrame()
  {
    tLayers::iterator    it( Layers.begin() );
    while ( it != Layers.end() )
    {
      delete *it;
      ++it;
    }
    Layers.clear();
  }
};



struct ImageSet
{
  typedef std::vector<tImageSetFrame*,GR::STL::global_allocator<tImageSetFrame*> >    tFrames;

  tFrames         Frames;

  int             LoopCount;   // 0 = endlos 



  ImageSet() :
    LoopCount( 0 )
  {
  }

  ~ImageSet()
  {
    tFrames::iterator    it( Frames.begin() );
    while ( it != Frames.end() )
    {
      delete *it;
      ++it;
    }
    Frames.clear();
  }



  tImageSetFrame* AddFrame( GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pDataMask = NULL )
  {
    Frames.push_back( new tImageSetFrame() );

    tImageSetFrame*   pFrame = Frames.back();

    tImageSetLayer*   pLayer = new tImageSetLayer();
    pLayer->pImageData      = pData;
    pLayer->pImageDataMask  = pDataMask;

    pFrame->Layers.push_back( pLayer );

    return pFrame;
  }



  GR::Graphic::ImageData* Image( size_t FrameIndex, size_t LayerIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    if ( LayerIndex >= Frames[FrameIndex]->Layers.size() )
    {
      return NULL;
    }
    return Frames[FrameIndex]->Layers[LayerIndex]->pImageData;
  }



  tImageSetFrame* Frame( size_t FrameIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    return Frames[FrameIndex];
  }



  GR::Graphic::Palette* Palette( size_t FrameIndex, size_t LayerIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    if ( LayerIndex >= Frames[FrameIndex]->Layers.size() )
    {
      return NULL;
    }
    return &Frames[FrameIndex]->Layers[LayerIndex]->pImageData->Palette();
  }



  GR::Graphic::ImageData* Mask( size_t FrameIndex, size_t LayerIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    if ( LayerIndex >= Frames[FrameIndex]->Layers.size() )
    {
      return NULL;
    }
    return Frames[FrameIndex]->Layers[LayerIndex]->pImageDataMask;
  }



  GR::Graphic::ImageData* DetachImage( size_t FrameIndex, size_t LayerIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    if ( LayerIndex >= Frames[FrameIndex]->Layers.size() )
    {
      return NULL;
    }
    GR::Graphic::ImageData*   pData = Frames[FrameIndex]->Layers[LayerIndex]->pImageData;
    Frames[FrameIndex]->Layers[LayerIndex]->pImageData = NULL;
    return pData;
  }



  GR::Graphic::ImageData* DetachMask( size_t FrameIndex, size_t LayerIndex )
  {
    if ( FrameIndex >= Frames.size() )
    {
      return NULL;
    }
    if ( LayerIndex >= Frames[FrameIndex]->Layers.size() )
    {
      return NULL;
    }
    GR::Graphic::ImageData*   pData = Frames[FrameIndex]->Layers[LayerIndex]->pImageDataMask;
    Frames[FrameIndex]->Layers[LayerIndex]->pImageDataMask = NULL;
    return pData;
  }



  size_t FrameCount() const
  {
    return Frames.size();
  }

};



#endif //_IMAGESET_H_