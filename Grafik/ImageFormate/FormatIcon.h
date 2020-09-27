#ifndef PLUGIN_ICON
#define PLUGIN_ICON



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatIcon : public AbstractImageFileFormat
    {
      public:

        FormatIcon();

        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String    GetFilterString();

    };


    extern FormatIcon    s_GlobalIconPlugin;

  }
}


#endif // PLUGIN_ICON
