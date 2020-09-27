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

        GR::Graphic::ImageData* m_pImageData;

        bool                    m_Interlaced;

        unsigned char           ucBuffer[2048],
                                GIFPass;

        long                    npixelrow;
        unsigned int            nnumbercolors;

        int                     nheight;                    // Image Höhe
        unsigned int            nwidth;                     // Image Breite
        unsigned long           npixelcolumn;

        short                   ncurcodesize;               // aktuelle Code-Size (max. 12bit)
        short                   nclearcode;                 // GIF ClearCode
        short                   nendofinformation;          // GIF End of Information Code EOI
        short                   nbottomslot;                // Erster verfügbare Code
        short                   ntopslot;                   // Höchstmöglicher Code für akt.
                                                            // Code-Size
        short                   nlastslot;                  // Der zuletzt gelesene Code
      
        // globale Definitionen für den Raster Data Block
        unsigned char           cdata[256];                 // Platz für Raster Data Block
        unsigned int            pdata;                      // Lese-Pointer in Buffer cData
        unsigned char           ccurrentbyte;               // aktuelles Byte aus cData
        short                   nbytesleft;                 // verbleibende Anzahl der Bytes in
                                                            // Buffer cData
        short                   nbitsleft;                  // verbleibende Anzahl der Bits in
                                                      // aktuellem Byte

        // nCodeMask wird für die Maskierung des Return-Codes auf
        // die aktuelle Code-Size in Modul GetCode() verwendet.
        long                    ncodemask[13];

        unsigned char           cpixelstack[4096];          // Pixelstack
        unsigned char           csuffix[4096];              // Suffix Tabelle mit Zeichen
        unsigned short          nprefix[4096];              // Prefix (verkettete Liste)
        unsigned char           coutbuffer[2049];           // Ausgabepuffer für Zeile
      
        HANDLE                  OutFile;                    // File to write to

        GR::u8                  Buffer[256];                // There must be one to much !!!
        int                     Index,                      // Current BYTE in buffer
                                BitsLeft;                   // Bits left to fill in current BYTE. These
                                                      // are right-justified

        int                     BitsPrPrimColor,            // Bits pr primary color
                                NumColors;                  // Number of colors in color table
        GR::u8                  ColorTable[768];
        GR::u16                 ScreenHeight,
                                ScreenWidth,
                                ImageHeight,
                                ImageWidth,
                                ImageLeft,
                                ImageTop,
                                RelPixX,
                                RelPixY;                    // Used by InputBYTE() -function

        int                     m_TempTransparentIndex,
                                m_FrameXOffset,
                                m_FrameYOffset,
                                m_FullWidth,
                                m_FullHeight;

        int                     GetNextByte();

        short int               DecodeImage( char* pTarget, const long Opacity );

        size_t                  ReadBlobBlock( GR::u8* data );

        GR::IO::FileStream*     m_pFile;

        GR::Graphic::ImageData* m_pSavingData;

        GR::IO::FileStream*     m_pSavingFile;



        int                     BitsNeeded( GR::u16 n );
        int                     WriteImageDescriptor( GR::IO::FileStream& MyFile, ImageDescriptor *id );
        int                     InputBYTE();
        int                     LZW_Compress(int codesize);

        void                    InitBitFile();
        int                     ResetOutBitFile();
        int                     WriteBits(int bits, int numbits);
        int                     AllocStrtab();
        GR::u16                 AddCharString( GR::u16 index, GR::u8 b );
        GR::u16                 FindCharString( GR::u16 index, GR::u8 b );
        void                    ClearStrtab(int codesize);
        void                    FreeStrtab();


        GR::u8*                 GIFStrChr;

        GR::u16*                StrNxt;
        GR::u16*                StrHsh;
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