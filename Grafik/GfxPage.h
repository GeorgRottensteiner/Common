#ifndef GFXPAGE_H
#define GFXPAGE_H

#include <GR/GRTypes.h>

#include <Grafik/Palette.h>



namespace GR
{
  namespace Graphic
  {


#define PAGE_TYPE_DIRECT_X        0x80000000L
#define PAGE_TYPE_555             0x40000000L
#define PAGE_TYPE_565             0x20000000L

#define COLOR_MASK_RED            0x7c00
#define COLOR_MASK_GREEN          0x03e0
#define COLOR_MASK_BLUE           0x001f


    class GFXPage
    {

      public:

        void*           pData;

        int             m_iXOffset,
                        m_iYOffset;

      protected:

        unsigned char   ucBitsPerPixel;

        unsigned long   ulType,
                        ulWidth,
                        ulHeight,
                        ulLineOffset,
                        ulBorderLeft,
                        ulBorderTop,
                        ulBorderRight,
                        ulBorderBottom;

        /*
        HWND            hWindow;

        bool            m_bUsedDC;

    #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        HDC             hDC;

        HPALETTE        hPalette;

        HBITMAP         hBitmap;

        HBITMAP         m_hOldBitmap;
    #endif*/

        // Helfer-Funktion für ClippedLine
        GR::u8            ComputeClipCode( int iX, int iY );


      public:

        GFXPage();
        ~GFXPage();

        //bool Create( HWND hwnd, unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp );
        bool Create( unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp );

        bool Destroy( void );

        void PutPixel( signed long slXPos, signed long slYPos, unsigned long ulColor, bool bIgnoreOffset = false );

        unsigned long GetPixel( signed long slXPos, signed long slYPos );

        void DarkenPixel( signed long slXPos, signed long slYPos, unsigned long ulFaktor );

        void LightenPixel( signed long slXPos, signed long slYPos, unsigned long ulFaktor );

        void ShadowPixel( signed long slXPos, signed long slYPos );

        void HLine( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor, bool bIgnoreOffset = false );

        void HLineAlpha( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor, unsigned char ucAlpha );

        void HLineAlpha50( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor );

        void ClippedLine( int iX1, int iY1, int iX2, int iY2, GR::u32 Color, bool bIgnoreOffset = false );

        void Line( signed long slXStart, signed long slYStart, signed long slXEnd,
                   signed long slYEnd, unsigned long ulColor, bool bIgnoreOffset = false );

        void Box( signed long slXStart, signed long slYStart, signed long slXEnd,
                  signed long slYEnd, unsigned long ulColor, bool bIgnoreOffset = false );

        void Rectangle( signed long slXStart, signed long slYStart, signed long slXEnd,
                        signed long slYEnd, unsigned long ulColor );

        void Ellipse( int iX, int iY, int iXRadius, int iYRadius, GR::u32 ucColor );
        void LargeEllipse( int iX, int iY, int iXRadius, int iYRadius, GR::u32 ucColor );
        void FilledEllipse( int iX, int iY, int iXRadius, int iYRadius, GR::u32 ucColor );

        inline void Circle( int iX, int iY, int iRadius, GR::u32 ucColor ) { Ellipse( iX, iY, iRadius, iRadius, ucColor ); }
        inline void LargeCircle( int iX, int iY, int iRadius, GR::u32 ucColor ) { LargeEllipse( iX, iY, iRadius, iRadius, ucColor ); }
        inline void FilledCircle( int iX, int iY, int iRadius, GR::u32 ucColor ) { FilledEllipse( iX, iY, iRadius, iRadius, ucColor ); }

        unsigned long GetType( void );

        int           GetWidth( void );

        int           GetHeight( void );

        unsigned char GetDepth( void );

        unsigned long GetLineOffset( void );

        unsigned long GetLeftBorder( void );

        unsigned long GetRightBorder( void );

        unsigned long GetTopBorder( void );

        unsigned long GetBottomBorder( void );

        void* GetData();

        /*
    #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        virtual HDC GetDC( void );

        virtual void ReleaseDC();

        HPALETTE GetPaletteObject( void );

        HBITMAP GetBitmapObject( void );
    #endif

        virtual HWND GetWindowObject( void );*/


        unsigned long GetRGB( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha = 0 );

        unsigned long GetRGB256( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha );
        unsigned long GetRGB256( GR::u32 Color );

        void SetType( unsigned long ulNewType );

        void SetLineOffset( unsigned long ulNewLineOffset );

        /*
    #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        void SetHDCObject( HDC hNewDC );
        void SetPaletteObject( HPALETTE hNewPalette );
        void SetBitmapObject( HBITMAP hNewBitmap );
    #endif*/

        void SetRange( int iLeft = -1, int iTop = -1,
                       int iRight = -1, int iBottom = -1 );

        void SetOffset( int iNewX, int iNewY );
        inline int GetOffsetX()
        {
          return m_iXOffset;
        }
        inline int GetOffsetY()
        {
          return m_iYOffset;
        }

        virtual void SetPalette( GR::Graphic::Palette* pPalette );

        virtual void GetPalette( GR::Graphic::Palette* pPalette );


        virtual void BeginAccess()
        {
        }

        virtual void EndAccess()
        {
        }

    };

  }

}

#endif // GFXPAGE_H



