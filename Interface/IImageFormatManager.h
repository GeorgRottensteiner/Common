#ifndef IIMAGEFORMATMANAGER_H
#define IIMAGEFORMATMANAGER_H



#include <map>
#include <string>

#include <Lang/Service.h>

#include <Grafik/Palette.h>

#include <Grafik\ImageData.h>
#include <Grafik\ImageFormate\ImageSet.h>

#include <Lang/GlobalAllocator.h>



class AbstractImageFileFormat;

class IImageFormatManager : public GR::Service::Service
{

  public:

    IImageFormatManager()
    {
      //dh::Log( "ImageFormatManager %x\n", this );
    }
    
    virtual ~IImageFormatManager()
    {
      //dh::Log( "~ImageFormatManager %x\n", this );
    }

    virtual GR::String                IdentifyType( const GR::String& FileName ) = 0;

    virtual GR::Graphic::ImageData*   LoadData( const GR::String& FileName, AbstractImageFileFormat** pPlugin = NULL ) = 0;
    virtual ImageSet*                 LoadSet( const GR::String& FileName, AbstractImageFileFormat** pPlugin = NULL ) = 0;
    virtual bool                      SaveData( const GR::String& FileName, GR::Graphic::ImageData* pData ) = 0;

};



#endif //IIMAGEFORMATMANAGER_H