#ifndef PLUGIN_JPG
#define PLUGIN_JPG



#define D_PROGRESSIVE_SUPPORTED
#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatJPG : public AbstractImageFileFormat
    {
      public:

        FormatJPG();
        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String    GetFilterString();
    
    };

  }
}



extern GR::Graphic::FormatJPG    globalJPGPlugin;

#endif // _PLUGIN_JPG_
