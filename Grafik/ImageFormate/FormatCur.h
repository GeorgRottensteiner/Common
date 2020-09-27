#ifndef PLUGIN_CURSOR
#define PLUGIN_CURSOR



#include <Grafik\ImageFormate\ImageFormatManager.h>



namespace GR
{
  namespace Graphic
  {
    class FormatCur : public AbstractImageFileFormat
    {
      public:

        FormatCur();

        virtual GR::String    GetDescription();
        virtual bool          IsFileOfType( const GR::String& FileName );
        virtual ImageSet*     LoadSet( const GR::String& FileName );
        virtual bool          CanSave( GR::Graphic::ImageData* pData );
        virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual GR::String    GetFilterString();

        virtual void          SetMetaData( const GR::String& Key, const GR::String& Value );
        virtual GR::String    MetaData( const GR::String& Key );

    };
  }
}


extern GR::Graphic::FormatCur    globalCursorPlugin;

#endif // PLUGIN_CUR
