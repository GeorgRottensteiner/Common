#ifndef PLUGIN_IGF
#define PLUGIN_IGF



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatIGF : public AbstractImageFileFormat
    {
      public:

        FormatIGF();

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



extern GR::Graphic::FormatIGF    globalIGFPlugin;

#endif // _PLUGIN_IGF_
