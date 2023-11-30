#pragma once


#include <Grafik\ImageFormate\ImageFormatManager.h>

#include <IO/FileChunk.h>



namespace GR
{
  namespace Graphic
  {
    class FormatGRI : public AbstractImageFileFormat
    {
      private:


        GR::IO::FileChunk FormatGRI::ImageToChunk( GR::Graphic::ImageData& ImageData, GR::u16 FileChunk, 
                                                   GR::Graphic::ImageData* pMaskData = NULL, GR::u16 FileChunkMask = 0,
                                                   GR::f32 FrameDelay = 0.0f );


      public:

        FormatGRI();

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



extern GR::Graphic::FormatGRI    globalGRIPlugin;

