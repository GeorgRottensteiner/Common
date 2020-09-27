#ifndef WING_PAGE_H
#define WING_PAGE_H

#include <GR/GRTypes.h>

#include <Grafik/Palette.h>

#include "GfxPage.h"

#include <windows.h>



namespace GR
{
  namespace Graphic
  {

#define PUSH_GFX_BEGIN ;
#define POP_GFX_DONE ;

    typedef struct struct_wing_palette
    {
      WORD              wVersion,
                        wNumberOfEntries;
      PALETTEENTRY      aEntry[256];
    } struct_wing_palette;



#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL )

    class GDIPage : public GFXPage
    {

      private:

        struct_wing_palette stPal;

        HWND            hWindow;

        bool            m_bUsedDC;

        HDC             hDC;

        HPALETTE        hPalette;

        HBITMAP         hBitmap;

        HBITMAP         m_hOldBitmap;



      public:

        GDIPage();
        ~GDIPage();

        bool Create( HWND hwnd, unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp );

        bool Destroy( void );

        bool GFXUpdate( bool bStretched, bool bMFC, HWND hwndMFC, HDC hdc, bool bForceUseHDC = FALSE );

        void SetPalette( GR::Graphic::Palette* pPalette );

        void GetPalette( GR::Graphic::Palette* pPalette );

        HDC GetDC();
        void ReleaseDC();

        void SetHDCObject( HDC hNewDC );
        void SetPaletteObject( HPALETTE hNewPalette );
        void SetBitmapObject( HBITMAP hNewBitmap );
        HPALETTE GetPaletteObject();
        HBITMAP GetBitmapObject();
        HWND GetWindowObject();

      };

#endif 

  }
}



#endif // __GFXPAGE_H__



