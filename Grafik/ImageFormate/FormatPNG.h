#ifndef PLUGIN_PNG
#define PLUGIN_PNG



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatPNG : public AbstractImageFileFormat
    {
      public:

      FormatPNG();

      virtual GR::String    GetDescription();
      virtual bool          IsFileOfType( const GR::String& FileName );
      virtual ImageSet*     LoadSet( const GR::String& FileName );
      virtual bool          CanSave( GR::Graphic::ImageData* pData );
      virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
      virtual GR::String    GetFilterString();

    };

    extern FormatPNG    s_GlobalPNGPlugin;
  }
}



#endif // PLUGIN_PNG
