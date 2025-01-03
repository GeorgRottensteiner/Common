#ifndef PLUGIN_IFF
#define PLUGIN_IFF



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatIFF : public AbstractImageFileFormat
    {
      public:

        class ChunkType
        {
          public:

            const static GR::u32 FORM = 0x4d524f46;
            const static GR::u32 ILBM = 0x4d424c49;
            const static GR::u32 BMHD = 0x44484d42;
            const static GR::u32 CMAP = 0x50414d43;
            const static GR::u32 BODY = 0x59444f42;
            const static GR::u32 CAMG = 0x474d4143;
            const static GR::u32 SPRT = 0x54525053;
            const static GR::u32 GRAB = 0x42415247;
            const static GR::u32 DEST = 0x54534544;
            const static GR::u32 CRNG = 0x474e5243;
            const static GR::u32 CCRT = 0x54524343;
        };



        FormatIFF();

        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          CanSave( ImageSet* pSet );

        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual bool          Save( const GR::String& FileName, ImageSet* pSet );
        virtual GR::String    GetFilterString();
    
    };

  }
}



extern GR::Graphic::FormatIFF    globalIFFPlugin;

#endif
