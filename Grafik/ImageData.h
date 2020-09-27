#ifndef IMAGEDATA_H
#define IMAGEDATA_H


#include <GR/GRTypes.h>
#include <Grafik/Palette.h>



namespace GR
{
  namespace Graphic
  {


    enum eImageFormat
    {
      IF_UNKNOWN      = 0,
      IF_PALETTED,
      IF_INDEX8 = IF_PALETTED,
      IF_A1R5G5B5,
      IF_X1R5G5B5,
      IF_R5G6B5,
      IF_R8G8B8,
      IF_X8R8G8B8,
      IF_A8R8G8B8,
      IF_A8,
      IF_A4R4G4B4,
      IF_COMPLEX,
      IF_INDEX4,
      IF_INDEX1,
      IF_MONOCHROME = IF_INDEX1,
      IF_INDEX2,
      IF_A2B10G10R10,
      IF_A2R10G10B10
    };

    class ImageData
    {

      protected:

        void*             m_pData;

        int               m_Width,
                          m_Height,
                          m_LineOffsetInBytes,
                          m_BytesPerLine;

        unsigned long     m_TransparentColor;

        GR::Graphic::eImageFormat      m_ImageFormat;

        GR::Graphic::Palette     m_Palette;

        bool              m_Owner,
                          m_TransparentColorUsed;


      public:


        ImageData();
        ImageData( const ImageData& rhsID );
        virtual ~ImageData();

        ImageData& operator= ( const ImageData& rhsID );

        int             BytesPerLine() const;
        int             LineOffsetInBytes() const;
        int             BitsProPixel() const;
        int             BytesProPixel() const;  // nur sinnvoll ab 8bit und mehr
        static int      BytesProPixel( GR::Graphic::eImageFormat Format );

        int             Width() const;
        int             Height() const;
        void*           Data() const;

        int             VirtualDataSize() const;
        int             DataSize() const;

        GR::Graphic::eImageFormat            ImageFormat() const;
        static GR::Graphic::eImageFormat     ImageFormatFromDepth( unsigned long dwDepth );
        static unsigned long    DepthFromImageFormat( GR::Graphic::eImageFormat );
        static bool             FormatHasAlpha( GR::Graphic::eImageFormat imgFormat );

        void*           GetRowData( int iY ) const;
        void*           GetRowColumnData( int iX, int iY ) const;
        GR::u32         GetPixel( int iX, int iY ) const;
        GR::u32         GetPixelRGB( int iX, int iY ) const;

        void            SetPalette( const GR::Graphic::Palette& Palette = GR::Graphic::Palette() );
        GR::Graphic::Palette&  Palette();
        const GR::Graphic::Palette&  Palette() const;
        bool            HasPaletteData() const;

        // in ein neues ImageData konvertieren (neues wird zurückgegeben)
        ImageData*      ConvertTo( GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes = 0, bool bColorKeying = false, GR::u32 TransparentColor = 0,
                                       int iX1 = 0, int iY1 = 0, int iWidth = 0, int iHeight = 0, GR::u32 ColorKeyReplacementColor = 0 );
        virtual bool    ConvertSelfTo( GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes = 0, bool bColorKeying = false, GR::u32 TransparentColor = 0,
                                   int iX1 = 0, int iY1 = 0, int iWidth = 0, int iHeight = 0, GR::u32 ColorKeyReplacementColor = 0 );
        // in ein bestehendes ImageData "hinein"-konvertieren
        bool            ConvertInto( ImageData* pTargetData, bool bColorKeying = false, GR::u32 TransparentColor = 0,
                                     int iX1 = 0, int iY1 = 0, int iWidth = 0, int iHeight = 0,
                                     GR::u32 ColorKeyReplacementColor  = 0 ) const;
        virtual void    Attach( int iWidth, int iHeight, int iLineOffsetInBytes, GR::Graphic::eImageFormat imageFormat, void* pData );
        virtual bool    CreateData( int iWidth, int iHeight, GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes = 0 );

        // damit SEHR vorsichtig sein, Owner deleten ihre Images bzw. ihr Data selbst
        void            SetOwnership( bool bOwner = true );

        void            TransparentColorUsed( bool bUsed );
        bool            TransparentColorUsed() const;

        void            TransparentColor( GR::u32 Color );
        unsigned long   TransparentColor() const;
   };

  }

}

#endif //_IMAGEDATA_H_