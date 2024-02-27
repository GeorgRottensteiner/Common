#ifndef CONTEXTDESCRIPTOR_H
#define CONTEXTDESCRIPTOR_H



#include <GR/GRTypes.h>

#include <Grafik/ImageData.h>



namespace GR
{
  namespace Graphic
  {
    class RLEList;
    class Palette;
    class Image;

    class GFXPage;

    class ContextDescriptor : public ImageData 
    {

      protected:

        bool                m_NothingVisible;


        void       _PutPixel1( int X, int Y, GR::u32 Color );
        void       _PutPixel2( int X, int Y, GR::u32 Color );
        void       _PutPixel4( int X, int Y, GR::u32 Color );
        void       _PutPixel8( int X, int Y, GR::u32 Color );
        void       _PutPixel555( int X, int Y, GR::u32 Color );
        void       _PutPixel565( int X, int Y, GR::u32 Color );
        void       _PutPixel24( int X, int Y, GR::u32 Color );
        void       _PutPixel32( int X, int Y, GR::u32 Color );

        void       _HLine1( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine2( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine4( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine8( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine16( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine24( int X1, int X2, int Y, GR::u32 Color );
        void       _HLine32( int X1, int X2, int Y, GR::u32 Color );

        GR::u32      _GetPixel1( int X, int Y ) const;
        GR::u32      _GetPixel2( int X, int Y ) const;
        GR::u32      _GetPixel4( int X, int Y ) const;
        GR::u32      _GetPixel8( int X, int Y ) const;
        GR::u32      _GetPixel555( int X, int Y ) const;
        GR::u32      _GetPixel565( int X, int Y ) const;
        GR::u32      _GetPixel24( int X, int Y ) const;
        GR::u32      _GetPixel32( int X, int Y ) const;

        void        _CopyPixel8( void* pDest, void* pSource );
        void        _CopyPixel16( void* pDest, void* pSource );
        void        _CopyPixel24( void* pDest, void* pSource );
        void        _CopyPixel32( void* pDest, void* pSource );

        void        _CopyPixelAlpha555( void* pDest, void* pSource, int Alpha );
        void        _CopyPixelAlpha565( void* pDest, void* pSource, int Alpha );
        void        _CopyPixelAlpha24( void* pDest, void* pSource, int Alpha );
        void        _CopyPixelAlpha32( void* pDest, void* pSource, int Alpha );

        void        _CopyPixelMask8( void* pDest, void* pSource, void* pMask );
        void        _CopyPixelMask555( void* pDest, void* pSource, void* pMask );
        void        _CopyPixelMask565( void* pDest, void* pSource, void* pMask );
        void        _CopyPixelMask24( void* pDest, void* pSource, void* pMask );
        void        _CopyPixelMask32( void* pDest, void* pSource, void* pMask );

        void        _CopyLineTransparent4( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget );
        void        _CopyLineTransparent8( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget );
        void        _CopyLineTransparent16( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget );
        void        _CopyLineTransparent24( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget );
        void        _CopyLineTransparent32( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget );

        void        _CopyAreaAlphaSlow( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlpha555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlpha565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlpha24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlpha32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );

        void        _CopyAreaAlphaAndMask4( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMask8( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMask555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMask565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMask24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMask32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );

        void        _CopyAreaAlphaAndMaskScaled555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMaskScaled24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaAndMaskScaled32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );

        void        _CopyAreaAlphaTransparent555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaTransparent565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaTransparent24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAreaAlphaTransparent32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget = NULL );

        void        _CopyAeraAlphaMaskWithColor555( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAeraAlphaMaskWithColor565( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAeraAlphaMaskWithColor24( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void        _CopyAeraAlphaMaskWithColor32( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );

        void        _AlphaBox16( int X1, int Y1, int Width, int Height, GR::u32 Color, int Alpha );

        void              _CopyLine1( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine2( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine4( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine8( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine16( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine24( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void              _CopyLine32( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );

        // interne Helper
        void              CopyPixel( void* pDest, void* pSource );
        void              CopyPixelAlpha( void* pDest, void* pSource, int Alpha );
        void              CopyPixelMask( void* pDest, void* pSource, void* pMask );

        void              AssignColorDepth();

      public:

        unsigned long     m_BitDepth;

        int               m_ClipLeft,
                          m_ClipTop,
                          m_ClipRight,
                          m_ClipBottom;

        int               m_XOffset;
        int               m_YOffset;

        typedef void (ContextDescriptor::* PUTPIXELPROC)( int, int, GR::u32 );
        PUTPIXELPROC        PutPixelProc;
        typedef void (ContextDescriptor::* HLINEPROC)( int, int, int, GR::u32 );
        HLINEPROC           HLineProc;
        typedef GR::u32 (ContextDescriptor::* GETPIXELPROC)( int, int ) const;
        GETPIXELPROC        GetPixelProc;
        typedef void (ContextDescriptor::* COPYPIXELPROC)( void*, void* );
        COPYPIXELPROC       CopyPixelProc;
        typedef void (ContextDescriptor::* COPYPIXELALPHAPROC)( void*, void*, int Alpha );
        COPYPIXELALPHAPROC  CopyPixelAlphaProc;
        typedef void (ContextDescriptor::* COPYPIXELMASKPROC)( void*, void*, void* );
        COPYPIXELMASKPROC   CopyPixelMaskProc;
        typedef void (ContextDescriptor::* COPYAREAALPHAPROC)( int, int, int, int, int, int, int, ContextDescriptor* );
        COPYAREAALPHAPROC   CopyAreaAlphaProc;
        typedef void (ContextDescriptor::* COPYAREAALPHAANDMASKPROC)( int, int, int, int, int, int, int, ContextDescriptor*, ContextDescriptor* );
        COPYAREAALPHAANDMASKPROC        CopyAreaAlphaAndMaskProc;
        typedef void (ContextDescriptor::* COPYAREAALPHAANDMASKSCALEDPROC)( int, int, int, int, int, int, int, int, int, ContextDescriptor*, ContextDescriptor* );
        COPYAREAALPHAANDMASKSCALEDPROC  CopyAreaAlphaAndMaskScaledProc;
        typedef void (ContextDescriptor::* COPYAREAALPHATRANSPARENTPROC)( int, int, int, int, int, int, int, GR::u32, ContextDescriptor* );
        COPYAREAALPHATRANSPARENTPROC    CopyAreaAlphaTransparentProc;
        typedef void (ContextDescriptor::* ALPHABOXPROC)( int, int, int, int, GR::u32, int );
        ALPHABOXPROC                    AlphaBoxProc;
        typedef void (ContextDescriptor::* COPYLINEPROC)( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget );
        COPYLINEPROC                    CopyLineProc;
        typedef void (ContextDescriptor::* COPYLINETRANSPARENTPROC)( int, int, int, int, int, GR::u32, ContextDescriptor* );
        COPYLINETRANSPARENTPROC         CopyLineTransparentProc;

        ContextDescriptor();
        ContextDescriptor( GR::Graphic::GFXPage* pPage, GR::Graphic::Palette* pPal = NULL );
        ContextDescriptor( GR::Graphic::Image* pImage, GR::Graphic::Palette* pPal = NULL );
        ContextDescriptor( GR::Graphic::Image& Image, int X = 0, int Y = 0, int Width = 0, int Height = 0, GR::Graphic::Palette* pPal = NULL );
        ContextDescriptor( const ImageData& ImageData );
        ContextDescriptor( const ContextDescriptor& ImageData );
        virtual ~ContextDescriptor();

        ContextDescriptor&       operator=( const ContextDescriptor& rhsCD );


        // Virtuelle Funktionen um ImageData drumrum
        void                      Clear();

        virtual bool Create( GFXPage *pPage, GR::Graphic::Palette *pPalette = NULL );
        virtual bool Create( GR::Graphic::Image *pImage, GR::Graphic::Palette *pPalette = NULL );
        virtual bool              CreateData( int Width, int Height, GR::Graphic::eImageFormat imageFormat, int LineOffsetInBytes = 0 );

        virtual void              Attach( int Width, int Height, int LineOffsetInBytes, GR::Graphic::eImageFormat imageFormat, void* pData );

        ContextDescriptor*        ConvertTo( unsigned long dwDepth, int LineOffsetInBytes = 0, bool ColorKeying = false, GR::u32 TransparentColor = 0,
                                             int X1 = 0, int Y1 = 0, int Width = 0, int Height = 0 );

        ContextDescriptor*        ConvertTo( GR::Graphic::eImageFormat imageFormat, int LineOffsetInBytes = 0, bool ColorKeying = false, GR::u32 TransparentColor = 0,
                                             int X1 = 0, int Y1 = 0, int Width = 0, int Height = 0 );

        bool                      ConvertSelfTo( GR::Graphic::eImageFormat imageFormat, int LineOffsetInBytes = 0, bool ColorKeying = false, GR::u32 TransparentColor = 0,
                                                 int X1 = 0, int Y1 = 0, int Width = 0, int Height = 0 );
        bool                      ConvertSelfTo( unsigned long dwDepth, int LineOffsetInBytes = 0, bool ColorKeying = false, GR::u32 TransparentColor = 0,
                                                 int X1 = 0, int Y1 = 0, int Width = 0, int Height = 0 );

        void                      SetClipRect( int X1, int Y1, int X2, int Y2 );
        void                      SetOffset( int X, int Y );

        // Direkt-Zugriff
        GR::u32                   GetDirectPixel( int X, int Y );
        void                      PutDirectPixel( int X, int Y, GR::u32 Color );

        size_t                    MapColorToPalette( GR::u32 Color );
        size_t                    MapColor( GR::u32 Color );

        // Schnellzugriff (ohne Bereichstest!!!) 
        void                      PutPixelFast( int X, int Y, GR::u32 Color );

        // indirekter Zugriff
        void                      PutPixel( int X, int Y, GR::u32 Color );
        void                      PutPixelAlpha( int X, int Y, GR::u32 Color, int Alpha );
        GR::u32                   GetPixel( int X, int Y ) const;

        void                      Line( int X1, int Y1, int X2, int Y2, GR::u32 Color );
        void                      AALine( int X1, int Y1, int X2, int Y2, GR::u32 Color );

        void                      HLine( int X1, int X2, int Y, GR::u32 Color );
        void                      Box( int X, int Y, int Width, int Height, GR::u32 Color );
        void                      Rectangle( int X, int Y, int Width, int Height, GR::u32 Color );
        void                      Ellipse( int x, int y, int a, int b, GR::u32 ucColor );
        void                      FilledEllipse( int x, int y, int a, int b, GR::u32 ucColor );
        void                      Circle( int x, int y, int Radius, GR::u32 ucColor );
        void                      FilledCircle( int x, int y, int Radius, GR::u32 ucColor );

        void                      CopyLine( int X1, int Y1, int Width, int ZX = 0, int ZY = 0, ContextDescriptor* pCDTarget = NULL );
        void                      AlphaBox( int X1, int Y1, int Width, int Height, GR::u32 Color, int Alpha );

        // CopyAreas
        void                      CopyLineTransparent( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget = NULL );

        void                      CopyArea( int X1, int Y1, int Width, int Height, int ZX = 0, int ZY = 0, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color = 0xffffffff, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaTransparent( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaScaled( int X1, int Y1, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaScaledTransparent( int X1, int Y1, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void                      CopyRotated( int X, int Y, float Angle, float ScaleX, float ScaleY, ContextDescriptor* pCDTarget = NULL );
        void                      CopyRotatedTransparent( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent = 0, ContextDescriptor* pCDTarget = NULL );
        void                      CopyRotatedAlpha( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void                      CopyRotatedAdditive( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent = 0, ContextDescriptor* pCDTarget = NULL );
        void                      CopyRotatedAdditiveColorized( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent = 0, GR::u32 Color = 0xffffffff, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlpha( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlphaTransparent( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent = 0, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlphaAndMask( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlphaAndMaskColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAlphaMaskWithColor( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlphaAndMaskScaled( int X, int Y, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget );
        void                      CopyAreaAdditive( int X, int Y, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAdditiveTransparent( int X, int Y, int Width, int Height, int ZX, int ZY, GR::u32 Transparent = 0, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAdditiveTransparentColorized( int X, int Y, int Width, int Height, int ZX, int ZY, GR::u32 Transparent = 0, ContextDescriptor* pCDTarget = NULL, GR::u32 Color = 0xffffffff );
        void                      CopyAreaAlphaAndMaskRotated( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaAlphaAndMaskRotatedColorized( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );

        void                      _CopyAreaAlphaAndMaskColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget );

        void                      CopyAreaSimple( int X1, int Y1, int Width, int Height, int ZX = 0, int ZY = 0, ContextDescriptor* pCDTarget = NULL );

        void                      MirrorH( int X, int Y, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget );
        void                      MirrorV( int X, int Y, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget );

        GR::Graphic::Image*       CreateImage();

        GR::Graphic::Image*       GetImage( const int PosX, 
                                            const int PosY,
                                            const int Width,
                                            const int Height );

        void                      CopyAreaCompressed( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::Graphic::RLEList& rleList, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );
        void                      CopyAreaCompressedColorize( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::Graphic::RLEList& rleList, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget = NULL );

        bool                      ReplaceColor( GR::u32 OldColor, GR::u32 NewColor );

    };

  }
}

#endif // __CONTEXTDESCRIPTOR_H_



