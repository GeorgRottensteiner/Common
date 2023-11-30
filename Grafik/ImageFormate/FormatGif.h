#ifndef PLUGIN_GIF
#define PLUGIN_GIF



#include <Grafik\ImageFormate\ImageFormatManager.h>

#include <IO/FileStream.h>



namespace GR
{
  namespace Graphic
  {
    class FormatGIF : public AbstractImageFileFormat
    {
      private:

        #pragma pack(1)
        struct ImageDescriptor
        {
          GR::u8          Separator;
          GR::u16         LeftPosition,
                          TopPosition;
          GR::u16         Width,
                          Height;
          GR::u8          LocalColorTableSize : 3,
                          Reserved            : 2,
                          SortFlag            : 1,
                          InterlaceFlag       : 1,
                          LocalColorTableFlag : 1;
        };

		    struct tGIFImageDescriptor
        {	
			    unsigned short  XPos;			    // Image Left Position
			    unsigned short  YPos;			    // Image Top Position
			    unsigned short  Width;			  // Image Width
			    unsigned short  Height;			  // Image Height
			    unsigned char   PackedFields;	// Packed Fields. Bits detail:
											                  //  0-2: Size of Local Color Table
											                  //  3-4: (Reserved)
											                  //    5: Sort Flag
											                  //    6: Interlace Flag
											                  //    7: Local Color Table Flag
		    };
        #pragma pack()

        bool                    m_Interlaced;

        unsigned char           m_InterlacedPass;

        int                     NumColors;
        int                     nheight;                    // Image Höhe
        unsigned int            nwidth;                     // Image Breite

        GR::u8                  Buffer[256];                // There must be one to much !!!
        int                     Index,                      // Current BYTE in buffer
                                BitsLeft;                   // Bits left to fill in current BYTE. These
                                                      // are right-justified

        GR::u8                  ColorTable[768];
        GR::u16                 ScreenHeight,
                                ScreenWidth,
                                ImageHeight,
                                ImageWidth,
                                ImageLeft,
                                ImageTop;                    // Used by InputBYTE() -function

        int                     m_TempTransparentIndex,
                                m_FrameXOffset,
                                m_FrameYOffset,
                                m_FullWidth,
                                m_FullHeight;

        int                     GetNextByte( IIOStream& IOIn );

        short int               DecodeImage( IIOStream& IOIn, char* pTarget, const long Opacity );

        size_t                  ReadBlobBlock( IIOStream& IOIn, GR::u8* data );

        GR::Graphic::ImageData* m_pSavingData;

        GR::IO::FileStream*     m_pSavingFile;



        int                     BitsNeeded( GR::u16 n );
        int                     WriteImageDescriptor( GR::IO::FileStream& MyFile, ImageDescriptor *id );
        int                     InputBYTE();
        int                     LZW_Compress(int codesize);

        void                    InitBitFile();
        int                     ResetOutBitFile();
        int                     WriteBits(int bits, int numbits);
        GR::u16                 AddCharString( GR::u16 index, GR::u8 b );
        GR::u16                 FindCharString( GR::u16 index, GR::u8 b );
        void                    ClearStrtab(int codesize);

        GR::u8                  StringBuffer[4096];
        GR::u16                 StringNext[4096];
        GR::u16                 StringHash[9973];
        GR::u16                 NumStrings;


      public:

        FormatGIF();

        virtual GR::String      GetDescription();
        virtual bool            IsFileOfType( const GR::String& FileName );
        virtual ImageSet*       LoadSet( const GR::String& FileName );
        virtual bool            CanSave( GR::Graphic::ImageData* pData );
        virtual bool            CanSave( ImageSet* pSet );
        virtual bool            Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL );
        virtual bool            Save( const GR::String& FileName, ImageSet* pSet );
        virtual bool            SaveAsGIF87( const GR::String& FileName, ImageSet* pSet );
        virtual GR::String      GetFilterString();
    
    };

  }
}



extern GR::Graphic::FormatGIF    globalGIFPlugin;

#endif // PLUGIN_GIF