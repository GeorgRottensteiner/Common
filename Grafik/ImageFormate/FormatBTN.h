#ifndef PLUGIN_BTN
#define PLUGIN_BTN



#include <Grafik\ImageFormate\ImageFormatManager.h>


namespace GR
{
  namespace Graphic
  {
    class FormatBTN : public AbstractImageFileFormat
    {
      public:

        FormatBTN();

        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String    GetFilterString();
    
    };
  }
}



extern GR::Graphic::FormatBTN    globalBTNPlugin;

#endif // PLUGIN_BTN
