#ifndef IMAGE_H
#define IMAGE_H

/*----------------------------------------------------------------------------+
 | BYTE : Type (see Defines for more Details)                                 |
 | BYTE : Bits per Pixel Achtung: 15 = 16Bit/555 Format und 16 = 16Bit/565 F. |
 | LONG : Anzahl der folgenden Datensätze                                     |
 |                                                                            |
 | ---Image---                                                                |
 | WORD : Breite                                                              |
 | WORD : Höhe                                                                |
 | LONG : Transparenzfarbwert                                                 |
 | BYTE : Daten                                                               |
 +----------------------------------------------------------------------------*/



#include <GR/GRTypes.h>

#include <Grafik/GfxPage.h>


struct IIOStream;



namespace GR
{
  namespace Graphic
  {

    namespace IGFType
    {
      enum Value
      {
        NOTHING                       = 0,
        IMAGE                         = 1,
        ANIMATION                     = 2,
        FONT                          = 3,
        PALETTE                       = 4,
        IMAGE_WITH_COMPRESSION_LIST   = 5,
        IMAGE_WITH_PALETTE            = 6,
        IMAGE_WITH_COMPRESSION_LIST_AND_PALETTE = 7,
        ANIMATION_EXTENDED            = 8   // with delay and type
      };
    }

#define IMAGE_FLAG_555            0x40000000L
#define IMAGE_FLAG_COMPRESSED     0x20000000L

#define IMAGE_METHOD_PLAIN        0x00000000L
#define IMAGE_METHOD_TRANSPARENT  0x00000001L
#define IMAGE_METHOD_MIRROR_H     0x00000002L
#define IMAGE_METHOD_MIRROR_V     0x00000004L
#define IMAGE_METHOD_COLOR        0x00000008L
#define IMAGE_METHOD_MASK         0x00000010L
#define IMAGE_METHOD_SCALE        0x00000020L
#define IMAGE_METHOD_SHADOW_MASK  0x00000040L
#define IMAGE_METHOD_ALPHA_MASK   0x00000100L
#define IMAGE_METHOD_ALPHA50      0x00000200L
#define IMAGE_METHOD_COMPRESSED   0x00000400L
#define IMAGE_METHOD_ALPHA_VALUE  0x00000800L
#define IMAGE_METHOD_OPTIMAL      0x00001000L

#define IMAGE_ALPHA50_MASK_555    0x7bde // 0111.1011.1101.1110
#define IMAGE_ALPHA50_MASK_565    0xf79e // 1111.0111.1001.1110

#define IMAGE_RED_MASK_555        0x7c00 // 0111.1100.0000.0000
#define IMAGE_GREEN_MASK_555      0x03e0 // 0000.0011.1110.0000
#define IMAGE_BLUE_MASK_555       0x001f // 0000.0000.0001.1111

#define IMAGE_RED_MASK_565        0xf800 // 1111.1000.0000.0000
#define IMAGE_GREEN_MASK_565      0x07e0 // 0000.0111.1110.0000
#define IMAGE_BLUE_MASK_565       0x001f // 0000.0000.0001.1111

#define IMAGE_RED_SHIFT_555       10
#define IMAGE_GREEN_SHIFT_555     5
#define IMAGE_BLUE_SHIFT_555      0

#define IMAGE_RED_SHIFT_565       11
#define IMAGE_GREEN_SHIFT_565     5
#define IMAGE_BLUE_SHIFT_565      0

#define MAX_COMPRESS_SECTOR       0x7f
#define FLAG_COMPRESS_TRANSPARENT 0x80



    class ImageData;

    class Image
    {

      private:

        unsigned char   BitsPerPixel;
        unsigned char*  pCompressList;

        unsigned char*  pData;

        GR::u16         Width,
                        Height;

        GR::u32         CompressListSize;

        unsigned long   Flags,
                        TransparentColor;

        void PutCompressed( GR::Graphic::GFXPage* pActualPage, 
                            GR::u8* pDestinationBase, 
                            GR::u8* pSourceBase,
                            const signed long XDelta, 
                            const signed long YDelta, 
                            const signed long XOffset, 
                            const signed long YOffset, 
                            const unsigned long ulStep ) const;

        void PutCompressedAlphaMask(  GR::Graphic::GFXPage* pActualPage, 
                                      GR::u8* pDestinationBase,
                                      GR::u8* pSourceBase,
                                      GR::u8* pMaskBase,
                                      const signed long XDelta, 
                                      const signed long YDelta, 
                                      const signed long XOffset, 
                                      const signed long YOffset, 
                                      const unsigned long Step, 
                                      Image* pMaskImage ) const;

        void PutCompressedAlpha(  GR::Graphic::GFXPage* pActualPage, 
                                  GR::u8* pDestinationBase,
                                  GR::u8* pSourceBase,
                                  const signed long XDelta, 
                                  const signed long YDelta, 
                                  const signed long XOffset, 
                                  const signed long YOffset, 
                                  const unsigned long ulStep, 
                                  Image* pValueImage ) const;

        void PutAlphaMaskSector( GR::u16* pDestination, 
                                 GR::u16* pSource,
                                 GR::u8* pMask,
                                 const unsigned long Length ) const;

        void PutAlphaSector( GR::u16* pDestination,
                             GR::u8* pSource,
                             const unsigned long Length, 
                             const unsigned long Alpha, 
                             const unsigned char Bpp ) const;

        void PutBlock(  GR::Graphic::GFXPage* pActualPage, 
                        GR::u8* pDestination,
                        GR::u8* pSource,
                        const signed long XDelta, 
                        const signed long YDelta, 
                        const unsigned long Step ) const;

        unsigned long GetFaktor( const unsigned char Bpp ) const;

        void WriteSector( unsigned char** pList, 
                          const unsigned long Size, 
                          const bool Transparent, 
                          const bool Write ) const;

      public:

        // Erzeugt ein Imageobjekt mit den angegebenen Werten
        Image(  const unsigned long NewWidth = 1, 
                const unsigned long NewHeight = 1, 
                const unsigned char Bpp = 8, 
                const unsigned long NewFlags = 0, 
                const unsigned long NewTransparent = 0 );

        // Erzeugt ein Imageobjekt aus den Daten der angegebenen Datei
        Image( const GR::Char* FileName );

        // Erzeugt ein Imageobjekt aus einem Stream
        Image( IIOStream& Stream );

        // Erzeugt ein Imageobjekt aus einem ImageData-Objekt
        Image( const ImageData& ImageData );

        //- CopyConstructor
        Image( const Image& Image );
    
        // Erzeugt eine Kopie des angegebenen Imageobjektes
        Image( const Image* pCopyImage );

        //- Zuweisungsoperator
        Image& operator=( const Image& Image );

        // Entfernt das Imageobjekt
        ~Image();

        // Setzt das Image auf Anfangswerte
        bool Set( const GR::u16 NewWidth,
                  const GR::u16 NewHeight,
                  const unsigned char Bpp, 
                  const unsigned long NewFlags, 
                  const unsigned long NewTransparent );

        void Reset();

        // Erzeugt ein Imageobjekt aus einem ImageData-Objekt
        ImageData* CreateImageData();

        // Lädt ein Image aus der angegebenen Datei ein
        bool Load( const GR::Char* FileName );

        // Lädt ein Image der angegebenen Tiefe von der aktuellen Position der angegebenen Datei
        bool LoadAt( IIOStream& Stream, const unsigned char Bpp );

        // Lädt ein Image aus dem Stream
        bool Load( IIOStream& Stream );

        // Lädt ein BTN/BTH/BTX der angegebenen Tiefe aus der angegebenen Datei
        bool LoadBTN( const GR::Char* FileName, const unsigned char Bpp );

        // Lädt ein BTN/BTH/BTX der angegebenen Tiefe aus der aktuellen Position der angegebenen Datei
        bool LoadBTNAt( IIOStream& Stream, const unsigned char Bpp );

        // Lädt ein TGA beliebiger Tiefe aus der angegebenen Datei
        bool LoadTGA( const GR::Char* FileName );

        // Speichert das Image in die angegebene Datei
        bool Save( const GR::Char* FileName, const bool SaveWithCompressList = false, GR::Graphic::Palette* pPal = NULL ) const ;

        // Speichert das Image an die aktuelle Position der Datei
        bool SaveAt( IIOStream& Stream ) const;

        // Kopiert einen Bereich der aktuellen Seite an der angegebenen Position in das Image
        bool GetImage( GFXPage* pPage,
                       const int XPos, 
                       const int YPos );

        // Kopiert einen Bereich der aktuellen Seite an der angegebenen Position in das Image
        bool GetNewImage( GFXPage* pActualPage,
                          const signed long XPos, 
                          const signed long YPos, 
                          const GR::u16 NewWidth,
                          const GR::u16 NewHeight );

        // Gibt einen Teil des Images als neues Image zurück
        Image* GetTileImage( const GR::u16 X,
                             const GR::u16 Y,
                             const GR::u16 Width,
                             const GR::u16 Height );

        // Zeigt das Image in dem gewünschten Modus an die angegebene Position in der aktuellen Seite an
        bool PutImage( GFXPage* pActualPage,
                       const signed long XPos, 
                       const signed long YPos, 
                       const unsigned long Flags = IMAGE_METHOD_OPTIMAL, 
                       Image* pMaskImage = NULL ) const;

        bool PutImageReplaceColor( GFXPage* pActualPage,
                                   const signed long XPos, 
                                   const signed long YPos, 
                                   GR::u32 Color1,
                                   GR::u32 Color2 ) const;

        bool PutImageScale( GFXPage* pActualPage,
                            signed long XPos, 
                            signed long YPos, 
                            const float* pScaleBuffer,
                            float ScaleOffset,
                            unsigned long ScaleSize,
                            unsigned long Flags = IMAGE_METHOD_OPTIMAL );

        // Gibt den Wert des Pixels an der angegebenen Position zurück
        unsigned long GetPixel( const unsigned long XPos, 
                                const unsigned long YPos ) const;
        void SetPixel( const unsigned long XPos, 
                       const unsigned long YPos,
                       unsigned long Color );


        // Konvertiert das Image in die angegebene Farbtiefe
        bool Convert( unsigned char Bpp, GR::Graphic::Palette* pPalette = NULL );

        // Komprimiert das Image
        unsigned long Compress( bool WriteList = true );

        // Gibt die Flags des Images zurück
        inline unsigned long GetFlags( void ) const { return Flags; }
    
        // Gibt die Breite des Images zurück
        inline GR::u16 GetWidth( void ) const { return Width; }

        // Gibt die Höhe des Images zurück
        inline GR::u16 GetHeight( void ) const { return Height; }

        // Gibt die Anzahl der Bits pro Pixel zurück
        inline unsigned char GetDepth( void ) const { return BitsPerPixel; }

        // Gibt die Größe des Images zurück
        unsigned long GetSize( void ) const;

        // Gibt die Größe einer Bild-Zeile zurück (in Bytes)
        unsigned long GetLineSize() const;

        // Gibt den Farbwert zurück, der für dieses Image transparent ist
        inline unsigned long GetTransparentColor( void ) const { return TransparentColor; }

        unsigned long GetTransparentColor32( GR::Graphic::Palette* pPal = NULL ) const;

        // Gibt den Zeiger auf die Daten des Images zurück
        inline void *GetData( void ) const {  return pData; }

        // Gibt den Zeiger auf die Komprimierungsliste zurück
        // Vorsicht: Bitte nicht benutzen!
        GR::u8* GetCompressList( void );

        // Gibt die Größe der Compress-List zurück
        inline unsigned long GetCompressListSize( void ) const { return CompressListSize; }

        // Setzt die Flags des Images
        inline void SetFlags( const unsigned long NewFlags ) { Flags = NewFlags; }

        // Setzt den Farbwert, der für dieses Image transparent ist
        void SetTransparentColor( const unsigned long NewTransparentColor ) { TransparentColor = NewTransparentColor; }
        void SetTransparentColorRGB( const unsigned long NewTransparentColor );

        bool PutAlphaMask( GFXPage* pActualPage,
                           const long Imagecolor,
                           const long Globalalpha,
                           const signed long YPos,
                           const signed long XPos ) const;

    };

  }

}


#endif // __IMAGE_H__



