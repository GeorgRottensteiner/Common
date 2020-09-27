#ifndef DDRAW_PAGE_H
#define DDRAW_PAGE_H

#include <windows.h>

#include <GR/GRTypes.h>

#include <Grafik/Palette.h>

#include "GfxPage.h"

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
#ifndef __NO_DIRECT_X__
#include <ddraw.h>
#endif
#endif

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

#ifndef __NO_DIRECT_X__
#else
#define PUSH_GFX_BEGIN ;
#define POP_GFX_DONE ;
#endif


#ifndef __NO_DIRECT_X__
    class DirectXPage : public GFXPage
    {

      private:

        static bool           s_DDInitialized;

        static LPDIRECTDRAW   s_lpDD;


        bool                  GFXPaletteChanged,
                              GFXDrawActive;

        GR::u8                GFXDrawBeenActive;

        HWND                  hWindow;


        unsigned char         Palette[768];

        LPDIRECTDRAWSURFACE   lpDDSPrimary,
                              lpDDSBack;

        LPDIRECTDRAWPALETTE   pDirectDrawPalette;



      public:

        // Erzeugt eine leere DirectX Grafikseite
        DirectXPage();

        // Entfernt die DirectX Grafikseite
        ~DirectXPage();

        bool Create( HWND hwnd, unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp );

        bool Destroy();

        void GFXBeginStack();
        void GFXDoneStack();

        bool GFXBegin();

        bool GFXDone();

        bool GFXUpdate();

        void SetPalette( GR::Graphic::Palette* pPalette );

        void GetPalette( GR::Graphic::Palette* pPalette );

        virtual void BeginAccess();
        virtual void EndAccess();

        HWND GetWindowObject();

    };
#endif //__NO_DIRECTX__

  }

}

#endif // DDRAW_PAGE_H

