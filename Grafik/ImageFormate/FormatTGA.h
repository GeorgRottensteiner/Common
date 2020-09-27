#ifndef PLUGIN_TGA
#define PLUGIN_TGA

#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatTGA : public AbstractImageFileFormat
    {
      public:

        FormatTGA();
        virtual GR::String      GetDescription();
        virtual bool            IsFileOfType( const GR::String& FileName );
        virtual ImageSet*       LoadSet( const GR::String& FileName );
        virtual bool            CanSave( GR::Graphic::ImageData* pData );
        virtual bool            Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String      GetFilterString();
    
    };

  }
}



extern GR::Graphic::FormatTGA    globalTGAPlugin;

#endif //_PLUGIN_TGA_