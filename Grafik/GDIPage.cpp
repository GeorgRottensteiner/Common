#include "Palette.h"
#include "GDIPage.h"
#include <Misc/Misc.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace Graphic
  {

#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL )
    GDIPage::GDIPage()
    {
      m_bUsedDC     = false;
      hWindow       = NULL;
      hDC           = NULL;
      hBitmap       = NULL;
      hPalette      = NULL;
      m_hOldBitmap  = NULL;

      unsigned long   i;

      HDC             Screen        = NULL;

      HPALETTE        ScreenPalette = NULL,
                      OldPal        = NULL;

      stPal.wVersion          = 0x0300;
      stPal.wNumberOfEntries  = 256;
      for ( i = 0; i < 256; i++ )
      {
        stPal.aEntry[i].peRed   = 0;
        stPal.aEntry[i].peGreen = 0;
        stPal.aEntry[i].peBlue  = 0;
        stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
      }
      Screen        = ::GetDC( NULL );
      ScreenPalette = CreatePalette( (LOGPALETTE *)&stPal );
      if ( ScreenPalette )
      {
        OldPal = SelectPalette( Screen, ScreenPalette, FALSE );
        RealizePalette( Screen );
        ScreenPalette = SelectPalette( Screen, OldPal, FALSE );
        DeleteObject( ScreenPalette );
      }
      if ( GetDeviceCaps( Screen, NUMCOLORS ) != -1 )
      {
        GetSystemPaletteEntries( Screen, 0, 10, stPal.aEntry );
        GetSystemPaletteEntries( Screen, 246, 10, stPal.aEntry + 246 );
      }
      ::ReleaseDC( NULL, Screen );
    }



    GDIPage::~GDIPage()
    {
      Destroy();
    }



    bool GDIPage::Create( HWND hwnd, unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp )
    {
      if ( ( ulNewWidth == 0 )
      &&   ( ulNewHeight == 0 ) )
      {
        // um evtl. Abbrüche zu vermeiden
        ulNewWidth = 1;
        ulNewHeight = 1;
      }

      unsigned long   i;

      BITMAPINFO      bminfo;

      if ( ( ucBpp == 1 )
      ||   ( ucBpp == 2 )
      ||   ( ucBpp == 4 )
      ||   ( ucBpp == 8 )
      ||   ( ucBpp == 15 )
      ||   ( ucBpp == 16 )
      ||   ( ucBpp == 24 )
      ||   ( ucBpp == 32 ) )
      {
        GFXPage::Create( ulNewWidth, ulNewHeight, ucBpp );
        hWindow = hwnd;
        bminfo.bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
        bminfo.bmiHeader.biPlanes       = 1;
        bminfo.bmiHeader.biBitCount     = ucBpp;
        if ( ucBpp == 15 )
        {
          bminfo.bmiHeader.biBitCount   = 16;
        }
        bminfo.bmiHeader.biCompression  = BI_RGB;
        bminfo.bmiHeader.biSizeImage    = 0;
        bminfo.bmiHeader.biClrUsed      = 0;
        bminfo.bmiHeader.biClrImportant = 0;
        bminfo.bmiHeader.biWidth        = ulNewWidth;
        bminfo.bmiHeader.biHeight       = - (signed long)ulNewHeight;   // minus, damit es TOP-DOWN ist

        SetHDCObject( CreateCompatibleDC( NULL ) );
        SetBitmapObject( CreateDIBSection( GetDC(), &bminfo, DIB_PAL_COLORS, &pData, NULL, 0 ) );
        m_hOldBitmap = (HBITMAP)SelectObject( GetDC(), GetBitmapObject() );
        if ( ucBpp == 1 )
        {
          stPal.wNumberOfEntries  = 2;
          for ( i = 0; i < 2; i++ )
          {
            stPal.aEntry[i].peRed   = 0;
            stPal.aEntry[i].peGreen = 0;
            stPal.aEntry[i].peBlue  = 0;
            stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
          }
          SetPaletteObject( CreatePalette( (LOGPALETTE far *)&stPal ) );
        }
        else if ( ucBpp == 2 )
        {
          stPal.wNumberOfEntries = 4;
          for ( i = 0; i < 4; i++ )
          {
            stPal.aEntry[i].peRed = 0;
            stPal.aEntry[i].peGreen = 0;
            stPal.aEntry[i].peBlue = 0;
            stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
          }
          SetPaletteObject( CreatePalette( ( LOGPALETTE far* ) & stPal ) );
        }
        else if ( ucBpp == 4 )
        {
          stPal.wNumberOfEntries  = 16;
          for ( i = 0; i < 16; i++ )
          {
            stPal.aEntry[i].peRed   = 0;
            stPal.aEntry[i].peGreen = 0;
            stPal.aEntry[i].peBlue  = 0;
            stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
          }
          SetPaletteObject( CreatePalette( (LOGPALETTE far *)&stPal ) );
        }
        else if ( ucBpp == 8 )
        {
          for ( i = 0; i < 10; i++ )
          {
            stPal.aEntry[i].peFlags = 0;
          }
          for ( i = 10; i < 246; i++ )
          {
            stPal.aEntry[i].peRed   = 0;
            stPal.aEntry[i].peGreen = 0;
            stPal.aEntry[i].peBlue  = 0;
            stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
          }
          for ( i = 246; i < 256; i++ )
          {
            stPal.aEntry[i].peFlags = 0;
          }
          SetPaletteObject( CreatePalette( (LOGPALETTE far *)&stPal ) );
        }
        else if ( ( ucBpp == 16 )
        ||        ( ucBpp == 15 ) )
        {
          // Windows benutzt immer 555!
          SetType( PAGE_TYPE_555 );
        }
        Box( 0, 0, ulWidth - 1, ulHeight - 1, 0 );
        return TRUE;
      }
      else
      {
        dh::Log( "GDIPage::Create Unsupported Bit Depth %d.", ucBpp );
      }

      return FALSE;
    }



    bool GDIPage::Destroy()
    {
      if ( hBitmap )
      {
        SelectObject( hDC, m_hOldBitmap );
        DeleteObject( hBitmap );
        hBitmap = NULL;
      }
      if ( hDC )
      {
        DeleteDC( hDC );
        hDC = NULL;
      }
      if ( hPalette )
      {
        DeleteObject( hPalette );
        hPalette = NULL;
      }
      hWindow = NULL;

      GFXPage::Destroy();

      return TRUE;
    }



    bool GDIPage::GFXUpdate( bool bStretched, bool bMFC, HWND hwndMFC, HDC hdc, bool bForceUseHDC )
    {
      PAINTSTRUCT           ps;

      unsigned long         ulWidth         = GetWidth(),
                            ulHeight        = GetHeight();

      signed long           slScrollWidth   = 0,
                            slScrollHeight  = 0;

      RECT                  rc;

      if ( ( !bMFC )
      &&   ( !bForceUseHDC ) )
      {
        hdc = BeginPaint( GetWindowObject(), &ps );
      }
      if ( GetPaletteObject() != NULL )
      {
        SelectPalette( hdc, GetPaletteObject(), FALSE );
        RealizePalette( hdc );
      }
      if ( bMFC )
      {
        GetClientRect( hwndMFC, &rc );
        //dh::Log( "MFCTest: W:%ld H:%ld RC.L:%ld RC.T:%ld RC.R:%ld RC.B:%ld ScrW:%ld ScrH:%ld SW:%ld SH:%ld.", ulWidth, ulHeight, rc.left, rc.top, rc.right, rc.bottom, GetScrollPos( hwndMFC, SB_HORZ ), GetScrollPos( hwndMFC, SB_VERT ), GetWidth(), GetHeight() );
        slScrollWidth = GetScrollPos( hwndMFC, SB_HORZ );
        slScrollHeight = GetScrollPos( hwndMFC, SB_VERT );
      }
      else
      {
        GetClientRect( GetWindowObject(), &rc );
        //dh::Log( "Test: W:%ld H:%ld RC.L:%ld RC.T:%ld RC.R:%ld RC.B:%ld ScrW:%ld ScrH:%ld SW:%ld SH:%ld.", ulWidth, ulHeight, rc.left, rc.top, rc.right, rc.bottom, GetScrollPos( GetWindowObject(), SB_HORZ ), GetScrollPos( GetWindowObject(), SB_VERT ), GetWidth(), GetHeight() );
        slScrollWidth = GetScrollPos( GetWindowObject(), SB_HORZ );
        slScrollHeight = GetScrollPos( GetWindowObject(), SB_VERT );
      }
      if ( !bStretched )
      {
        if ( (signed long)ulWidth > rc.right - rc.left + 1 )
        {
          ulWidth = rc.right - rc.left + 1;
        }
        if ( (signed long)ulHeight > rc.bottom - rc.top + 1 )
        {
          ulHeight = rc.bottom - rc.top + 1;
        }
        //BitBlt( hdc, rc.left, rc.top, ulWidth, ulHeight, GetHDCObject(), slScrollWidth, slScrollHeight, SRCCOPY );
        BitBlt( hdc, slScrollWidth, slScrollHeight, ulWidth, ulHeight, GetDC(), slScrollWidth, slScrollHeight, SRCCOPY );
        //dh::Log( "Blit: RC.L:%ld RC.T:%ld W:%ld H:%ld ScrW:%ld ScrH:%ld.", rc.left, rc.top, ulWidth, ulHeight, slScrollWidth, slScrollHeight );
      }
      else
      {
        SetStretchBltMode( hdc, COLORONCOLOR );
        StretchBlt( hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                    GetDC(), 0, 0, GetWidth(), GetHeight(), SRCCOPY );
      }
      if ( ( !bMFC )
      &&   ( !bForceUseHDC ) )
      {
        EndPaint( GetWindowObject(), &ps );
      }

      return TRUE;
    }



    void GDIPage::SetPalette( GR::Graphic::Palette *pPalette )
    {
      if ( ucBitsPerPixel > 8 )
      {
        return;
      }

      unsigned long   iIndices = 1 << ucBitsPerPixel;
      unsigned long   i;

      RGBQUAD         buffer[256];

      for ( i = 0; i < iIndices; i++ )
      {
        stPal.aEntry[i].peRed   = buffer[i].rgbRed    = ( *( pPalette->Data() +     i * 3 ) );
        stPal.aEntry[i].peGreen = buffer[i].rgbGreen  = ( *( pPalette->Data() + 1 + i * 3 ) );
        stPal.aEntry[i].peBlue  = buffer[i].rgbBlue   = ( *( pPalette->Data() + 2 + i * 3 ) );
        stPal.aEntry[i].peFlags = PC_NOCOLLAPSE;
        buffer[i].rgbReserved   = 0;
      }
      SetDIBColorTable( GetDC(), 0, iIndices, buffer );
      SetPaletteObject( CreatePalette( (LOGPALETTE far *)&stPal ) );
    }



    void GDIPage::GetPalette( GR::Graphic::Palette *pPalette )
    {
      if ( ucBitsPerPixel > 8 )
      {
        return;
      }

      unsigned long   iIndices = 1 << ucBitsPerPixel;
      unsigned long   i;

      for ( i = 0; i < iIndices; i++ )
      {
        *( pPalette->Data() + i * 3     ) = stPal.aEntry[i].peRed;
        *( pPalette->Data() + i * 3 + 1 ) = stPal.aEntry[i].peGreen;
        *( pPalette->Data() + i * 3 + 2 ) = stPal.aEntry[i].peBlue;
      }
    }



    HDC GDIPage::GetDC()
    {
      return hDC;
    }



    void GDIPage::ReleaseDC()
    {
    }



    HPALETTE GDIPage::GetPaletteObject()
    {
      return hPalette;
    }



    HBITMAP GDIPage::GetBitmapObject()
    {
      return hBitmap;
    }



    void GDIPage::SetHDCObject( HDC hNewDC )
    {
      if ( hDC )
      {
        DeleteDC( hDC );
      }
      hDC = hNewDC;
    }



    void GDIPage::SetPaletteObject( HPALETTE hNewPalette )
    {
      if ( hPalette )
      {
        DeleteObject( hPalette );
      }
      hPalette = hNewPalette;
    }



    void GDIPage::SetBitmapObject( HBITMAP hNewBitmap )
    {
      if ( hBitmap )
      {
        DeleteObject( hBitmap );
      }
      hBitmap = hNewBitmap;
    }



    HWND GDIPage::GetWindowObject()
    {
      return hWindow;
    }

#endif

  }

}
