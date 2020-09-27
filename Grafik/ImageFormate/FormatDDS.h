#ifndef PLUGIN_DDS
#define PLUGIN_DDS



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatDDS : public AbstractImageFileFormat
    {
      public:

        FormatDDS();

        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String    GetFilterString();
    };

  }
}



extern GR::Graphic::FormatDDS    globalDDSPlugin;

#endif // PLUGIN_DDS
