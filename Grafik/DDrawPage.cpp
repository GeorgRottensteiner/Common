#include <Grafik/Palette.h>
#include <Grafik/DDrawPage.h>
#include <Misc/Misc.h>

#include <debug/debugclient.h>



#ifndef __NO_DIRECT_X__
#pragma comment( lib, "ddraw.lib" )
#endif //__NO_DIRECT_X__



#ifndef __NO_DIRECT_X__
static HRESULT( FAR WINAPI * MyDirectDrawCreate ) ( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lps_lpDD,    IUnknown FAR *pUnkOuter  ); 
#endif //__NO_DIRECT_X__


namespace GR
{
  namespace Graphic
  {

    #ifndef __NO_DIRECT_X__
    bool                    DirectXPage::s_DDInitialized = false;

    LPDIRECTDRAW            DirectXPage::s_lpDD = NULL;



    DirectXPage::DirectXPage()
    {
      HRESULT         ddrval;

      lpDDSBack = NULL;
      if ( !s_DDInitialized )
      {
        GFXDrawActive       = false;
        GFXPaletteChanged   = false;
        GFXDrawBeenActive   = 0;

        // DirectDraw-Objekt anlegen
        ddrval = DirectDrawCreate( NULL, &s_lpDD, NULL );
        if ( ddrval != DD_OK )
        {
          MessageBox( NULL, _T( "Couldn´t create DirectDraw Object." ), _T( "Fehler/Error" ), MB_OK | MB_APPLMODAL );
        }
        s_DDInitialized = true;
      }

    }



    DirectXPage::~DirectXPage()
    {

      Destroy();

    }



    bool DirectXPage::Create( HWND hwnd, unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp )
    {

      if ( ( ucBpp != 8 )
      &&   ( ucBpp != 15 )
      &&   ( ucBpp != 16 )
      &&   ( ucBpp != 24 )
      &&   ( ucBpp != 32 ) )
      {
        dh::Log( "Fehler: DirectXPage.Create Unsupported Bit Depth." );
        return FALSE;
      }

      unsigned long   i;

      HRESULT         ddrval;

      PALETTEENTRY    temp[256];

      DDSURFACEDESC   ddsd;

      DDCAPS_DX3      ddHALCaps;

      DDSCAPS         ddscaps;


      ddrval = s_lpDD->SetCooperativeLevel( hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
      if ( ddrval != DD_OK )
      {
        //LastError( "Fehler: DirectXPage.Create Couldn´t set cooperative level." );
        return FALSE;
      }

      ddrval = s_lpDD->SetDisplayMode( ulNewWidth, ulNewHeight, ucBpp );
      if ( ddrval != DD_OK )
      {
        s_lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
        //LastError( "Fehler: DirectXPage.Create Couldn´t set display mode." );
        return FALSE;
      }

      // Zweiten Puffer anlegen
      memset( &ddsd, 0, sizeof( ddsd ) );
      ddsd.dwSize             = sizeof( ddsd );
      ddsd.dwFlags            = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
      ddsd.ddsCaps.dwCaps     = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
      ddsd.dwBackBufferCount  = 1;
      ddrval                  = s_lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
      if ( ddrval != DD_OK )
      {
        s_lpDD->RestoreDisplayMode();
        s_lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
        //LastError( "Fehler: DirectXPage.Create Couldn´t set second Buffer." );
        return FALSE;
      }

      ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
      ddrval = lpDDSPrimary->GetAttachedSurface( &ddscaps, &lpDDSBack );
      if ( ddrval != DD_OK )
      {
        lpDDSPrimary->Release();
        s_lpDD->RestoreDisplayMode();
        s_lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
        //LastError( "Fehler: DirectXPage.Create Couldn´t get attached surface." );
        return FALSE;
      }


      // damit mit DX5 kompilierte Programme auch auf DX3 noch laufen
      ddHALCaps.dwSize = sizeof( DDCAPS_DX3 );
      ddrval = s_lpDD->GetCaps( (DDCAPS *)&ddHALCaps, NULL );
      if ( ddrval != DD_OK )
      {
        lpDDSPrimary->Release();
        s_lpDD->RestoreDisplayMode();
        s_lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
        //LastError( "Fehler: DirectXPage.Create Couldn´t get caps." );
        return FALSE;
      }

      // Spezialfall HiColor, rausfinden, ob eine 555 oder 565-Variante vorliegt
      if ( ( ucBpp == 15 )
      ||   ( ucBpp == 16 ) )
      {
        // dieser Fall MUSS vor CREATE aufgerufen werden!
        // (damit er je nach DirectDraw-Tiefe (555 oder 565) die richtigen Werte benutzt
        memset( &ddsd, 0, sizeof( ddsd ) );
        ddsd.dwSize = sizeof( ddsd );
        s_lpDD->GetDisplayMode( &ddsd );
        if ( ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB )
        {
          if ( ddsd.ddpfPixelFormat.dwGBitMask == 0x07e0 )
          {
            // wir haben ein 565-Pixel-Format!
            SetType( PAGE_TYPE_565 | PAGE_TYPE_DIRECT_X );
            ucBpp = 16;
          }
          else
          {
            // 555!
            SetType( PAGE_TYPE_555 | PAGE_TYPE_DIRECT_X );
            ucBpp = 15;
          }
        }
        else
        {
          // unbekannt? wir vermuten 555!
          SetType( PAGE_TYPE_555 | PAGE_TYPE_DIRECT_X );
          ucBpp = 15;
        }
      }

      // jetzt die Seite anlegen
      GFXPage::Create( ulNewWidth, ulNewHeight, ucBpp );

      if ( ucBpp == 8 )
      {
        // Paletten werden nur bei 8bit benötigt
        for ( i = 0; i < 256; i++ )
        {
          Palette[i * 3]      = (BYTE)i;
          Palette[i * 3 + 1]  = (BYTE)0;
          Palette[i * 3 + 2]  = (BYTE)i;
          temp[i].peRed       = (BYTE)i;
          temp[i].peGreen     = (BYTE)0;
          temp[i].peBlue      = (BYTE)i;
          temp[i].peFlags     = (BYTE)0;
        }
        pDirectDrawPalette = NULL;
        ddrval = s_lpDD->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                                      &temp[0],
                                      &pDirectDrawPalette,
                                      NULL );
        if ( ddrval != DD_OK )
        {
          CloseWindow( hwnd );
          //LastError( "Fehler: DirectXPage.Create Couldn´t create palette." );
          return FALSE;
        }
        ddrval = lpDDSPrimary->SetPalette( pDirectDrawPalette );
        if ( ddrval != DD_OK )
        {
          CloseWindow( hwnd );
          //LastError( "Fehler: DirectXPage.Create Couldn´t set palette." );
          return FALSE;
        }
      }
      return TRUE;
    }



    bool DirectXPage::Destroy()
    {
      if ( s_lpDD )
      {
        s_lpDD->SetCooperativeLevel( GetWindowObject(), DDSCL_NORMAL );
        s_lpDD->RestoreDisplayMode();
        s_lpDD->Release();
        s_lpDD = NULL;
      }

      s_DDInitialized = FALSE;

      GFXPage::Destroy();

      return TRUE;
    }



    bool DirectXPage::GFXBegin()
    {
      DDSURFACEDESC       ddsd;

      HRESULT             ret;

      RECT                rc;

      if ( !GFXDrawActive )
      {
        rc.left         = 0;
        rc.top          = 0;
        rc.right        = GetWidth() - 1;
        rc.bottom       = GetHeight() - 1;
        ddsd.dwSize     = sizeof( ddsd );
        ret = DDERR_WASSTILLDRAWING;
        while ( ret == DDERR_WASSTILLDRAWING )
        {
          ret = lpDDSBack->Lock( &rc, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL );
          if ( ret == DDERR_SURFACELOST )
          {
            // wichtig nach Alt-Tab
            ret = lpDDSPrimary->Restore();
            if ( ret == DD_OK )
            {
              ret = lpDDSBack->Restore();
              if ( ret == DD_OK )
              {
                ret = DDERR_WASSTILLDRAWING;
              }
              else
              {
                dh::Log( "Fehler: DirectXPage.GFXBegin Couldn´t restore back Screen." );
                return FALSE;
              }
            }
            else
            {
              //dh::Log( "Fehler: DirectXPage.GFXBegin Couldn´t restore primary Screen." );
              return FALSE;
            }
          }
        }
        if ( ret == DD_OK )
        {
          pData = (unsigned char *)ddsd.lpSurface;
          SetLineOffset( ddsd.lPitch );
          GFXDrawActive = TRUE;
          return TRUE;
        }
        else
        {
          dh::Log( "Fehler: DirectXPage.GFXBegin Irgendwas ist nicht in Ordnung." );
          return FALSE;
        }
      }
      return TRUE;
    }



    bool DirectXPage::GFXDone()
    {
      if ( GFXDrawActive )
      {
        lpDDSBack->Unlock( NULL );
        GFXDrawActive = FALSE;
      }
      return TRUE;
    }



    void DirectXPage::GFXBeginStack()
    {
      if ( GFXDrawActive )
      {
        GFXDrawBeenActive = 1;
      }
      else
      {
        GFXBegin();
        GFXDrawBeenActive = 2;
      }
    }



    void DirectXPage::GFXDoneStack()
    {
      if ( GFXDrawBeenActive == 2 )
      {
        GFXDone();
      }
      GFXDrawBeenActive = 0;
    }



    bool DirectXPage::GFXUpdate()
    {
      HRESULT               ddrval;

      PALETTEENTRY          temp[256];

      unsigned long         i;

      if ( GFXPaletteChanged )
      {
        GFXPaletteChanged = FALSE;
        for ( i = 0; i < 256; i++ )
        {
          temp[i].peRed   = (BYTE)( Palette[i * 3 + 0] );
          temp[i].peGreen = (BYTE)( Palette[i * 3 + 1] );
          temp[i].peBlue  = (BYTE)( Palette[i * 3 + 2] );
          temp[i].peFlags = 0;  //PC_NOCOLLAPSE;
        }
        ddrval = pDirectDrawPalette->SetEntries( 0, 0, 256, temp );
      }
      while( 1 )
      {
        // Solange loopen, bis der Flip hinhaut!
        ddrval = lpDDSPrimary->Flip( NULL, 0 );
        if ( ddrval == DD_OK )
        {
          // Geflipt, alles klar
          break;
        }
        if ( ddrval == DDERR_SURFACELOST )
        {
          // alles zurück
          ddrval = lpDDSPrimary->Restore();
          if ( ddrval == DD_OK )
          {
            ddrval = lpDDSBack->Restore();
            if ( ddrval != DD_OK )
            {
              dh::Log( "Fehler: DirectXPage.GFXUpdate Couldn´t restore back Screen." );
              return FALSE;
            }
          }
          else
          {
            //dh::Log( "Fehler: DirectXPage.GFXUpdate Couldn´t restore primary Screen." );
            return FALSE;
          }
        }
        if ( ddrval != DDERR_WASSTILLDRAWING )
        {
          dh::Log( "Fehler: DirectXPage.GFXUpdate Irgendwas ist nicht in Ordnung." );
          return FALSE;
        }
      }
      return TRUE;
    }



    void DirectXPage::SetPalette( GR::Graphic::Palette *pPalette )
    {
      for ( int i = 0; i < 256; i++ )
      {
        Palette[i * 3]     = *( pPalette->Data() + i * 3     );
        Palette[i * 3 + 1] = *( pPalette->Data() + i * 3 + 1 );
        Palette[i * 3 + 2] = *( pPalette->Data() + i * 3 + 2 );
      }
      GFXPaletteChanged = TRUE;
    }



    void DirectXPage::GetPalette( GR::Graphic::Palette *pPalette )
    {
      unsigned long   i;

      for ( i = 0; i < 256; i++ )
      {
        *( pPalette->Data() + i * 3     ) = Palette[i * 3];
        *( pPalette->Data() + i * 3 + 1 ) = Palette[i * 3 + 1];
        *( pPalette->Data() + i * 3 + 2 ) = Palette[i * 3 + 2];
      }
    }



    void DirectXPage::BeginAccess()
    {
      if ( GetType() & PAGE_TYPE_DIRECT_X )
      {
        GFXBeginStack();
      }
    }



    void DirectXPage::EndAccess()
    {
      if ( GetType() & PAGE_TYPE_DIRECT_X )
      {
        GFXDoneStack();
      }
    }



    HWND DirectXPage::GetWindowObject() 
    {
      return hWindow;
    }


#endif //__NO_DIRECT_X__

  }

}
