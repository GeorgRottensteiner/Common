#include <windows.h>
#include <commctrl.h>

#pragma warning( disable:4786 )
#include <map>

#include <Debug\debugclient.h>

#include <ControlHelper\CToolBar.h>

#include <ControlHelper\CDialog.h>



namespace WindowsWrapper
{

  CToolBar::CToolBar()
  {

    m_hbmImageWell = NULL;

    // default image sizes
    m_sizeImage.cx = 16;
    m_sizeImage.cy = 15;

    // default button sizes
    m_sizeButton.cx = 23;
    m_sizeButton.cy = 22;

  }



  CToolBar::~CToolBar()
  {

    if ( m_hbmImageWell )
    {
      DeleteObject( m_hbmImageWell );
    }

  }



  BOOL CToolBar::Create( HWND hwndParent,
                         DWORD dwStyle,
                         DWORD_PTR dwID )

  {
    RECT      rc;

    rc.left = 0;
    rc.top = 0;
    rc.right = 100;
    rc.bottom = 20;

    BOOL    bResult = CWnd::CreateEx( 0,
                                      TOOLBARCLASSNAMEA,
                                      NULL,
                                      dwStyle,
                                      0, 0,
                                      0, 0,
                                      hwndParent,
                                      (UINT_PTR)dwID,
                                      NULL );

    if ( bResult )
    {
      //AdjustParentClientRect( hwndParent );
    }
    return bResult;
  }



  BOOL CToolBar::CreateEx( HWND hwndParent, DWORD dwCtrlStyle, DWORD dwStyle, DWORD_PTR dwID )
  {
    RECT      rc;

    rc.left = 0;
    rc.top = 0;
    rc.right = 100;
    rc.bottom = 20;

    dwStyle |= dwCtrlStyle;

    return CWnd::Create( TOOLBARCLASSNAMEA, "", dwStyle, rc, hwndParent, dwID );
  }



  void CToolBar::AdjustParentClientRect( HWND hwndParent )
  {
    RECT    rcClientOld; // Old Client Rect
    RECT	  rcClientNew; // New Client Rect with Tollbar Added

    ::GetClientRect( hwndParent, &rcClientOld ); // Retrive the Old Client WindowSize

    // Called to reposition and resize control bars in the client 
    // area of a window. The reposQuery FLAG does not really traw the 
    // Toolbar.  It only does the calculations. And puts the new 
    // ClientRect values in rcClientNew so we can do the rest of the 
    // Math.
    /*
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST,
     AFX_IDW_CONTROLBAR_LAST,0,reposQuery,rcClientNew);
    */

    RECT    myRC;

    GetClientRect( &myRC );

    rcClientNew = rcClientOld;
    rcClientNew.top += myRC.bottom - myRC.top;
    //rcClientNew.bottom += myRC.bottom - myRC.top;

    // All of the Child Windows (Controls) now need to be moved so 
    // the Toolbar does not cover them up. Offest to move all child 
    // controls after adding Tollbar
    POINT     ptOffset;

    ptOffset.x = rcClientNew.left - rcClientOld.left;
    ptOffset.y = rcClientNew.top - rcClientOld.top;

    /*
    RECT      rcChild;

    // Handle to the Dialog Controls
    HWND  hwndChild = ::GetWindow( hwndParent, GW_CHILD );
    while( hwndChild ) // Cycle through all child controls
    {
      if ( hwndChild != m_hWnd )
      {
        // alle Childs außer dem Toolbar nach unten schieben
        ::GetWindowRect( hwndChild, &rcChild );
        ScreenToClient( &rcChild );

      // Changes the Child Rect by the values of the claculated offset
       ::OffsetRect( &rcChild, ptOffset.x, ptOffset.y );
       ::MoveWindow( hwndChild, rcChild.left, rcChild.top, rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, FALSE );
      }
      hwndChild = ::GetNextWindow( hwndChild, GW_CHILD );
    }
    */

    RECT	rcWindow;
    ::GetWindowRect( hwndParent, &rcWindow ); // Get the RECT of the Dialog

    // Increase width to new Client Width
    rcWindow.right += ( rcClientOld.right - rcClientOld.left ) - ( rcClientNew.right - rcClientNew.left );

    // Increase height to new Client Height
    rcWindow.bottom += ( rcClientOld.bottom - rcClientOld.top ) - ( rcClientNew.bottom - rcClientNew.top );

    //::MoveWindow( m_hWnd, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, FALSE ); // Redraw Window

    /*
    CDialog   theDialog;

    theDialog.Attach( hwndParent );
    ::SetWindowPos( hwndParent, NULL, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_FRAMECHANGED | SWP_NOZORDER );

    ::InvalidateRect( hwndParent, NULL, TRUE );
    */

  }



  /*-LoadToolBar----------------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/

  BOOL CToolBar::LoadToolBar( DWORD dwResourceId )
  {

    return LoadToolBar( MAKEINTRESOURCE( dwResourceId ) );

  }



  struct CToolBarData
  {
    WORD wVersion;
    WORD wWidth;
    WORD wHeight;
    WORD wItemCount;
    //WORD aItems[wItemCount]

    WORD* items()
    {
      return (WORD*)( this + 1 );
    }
  };

  /*-LoadToolBar----------------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/

  BOOL CToolBar::LoadToolBar( LPCTSTR lpszResourceName )
  {

    // determine location of the bitmap in resource fork
    HINSTANCE   hInstance = GetModuleHandle( NULL );
    HRSRC hRsrc = ::FindResource( hInstance, lpszResourceName, MAKEINTRESOURCE( 241 ) );
    if ( hRsrc == NULL )
    {
      return FALSE;
    }

    HGLOBAL hGlobal = LoadResource( hInstance, hRsrc );
    if ( hGlobal == NULL )
    {
      return FALSE;
    }

    CToolBarData* pData = (CToolBarData*)LockResource( hGlobal );
    if ( pData == NULL )
    {
      return FALSE;
    }

    UINT* pItems = new UINT[pData->wItemCount];
    for ( int i = 0; i < pData->wItemCount; i++ )
    {
      pItems[i] = pData->items()[i];
    }
    BOOL bResult = SetButtons( pItems, pData->wItemCount );
    delete[] pItems;


    if ( bResult )
    {
      // set new sizes of the buttons
      SendMessage( TB_SETBITMAPSIZE, 0, MAKELONG( pData->wWidth, pData->wHeight ) );
      SendMessage( TB_SETBUTTONSIZE, 0, MAKELONG( pData->wWidth + 7, pData->wHeight + 7 ) );

      Invalidate();   // just to be nice if called when toolbar is visible

      // load bitmap now that sizes are known by the toolbar control
      bResult = LoadBitmap( lpszResourceName );
    }

    UnlockResource( hGlobal );
    FreeResource( hGlobal );

    return bResult;
  }


#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

  struct AFX_COLORMAP
  {
    // use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
    DWORD rgbqFrom;
    int iSysColorTo;
  };

  static const AFX_COLORMAP _afxSysColorMap[] =
  {
    // mapping from color in DIB to system color
    { RGB_TO_RGBQUAD( 0x00, 0x00, 0x00 ),  COLOR_BTNTEXT },       // black
    { RGB_TO_RGBQUAD( 0x80, 0x80, 0x80 ),  COLOR_BTNSHADOW },     // dark gray
    { RGB_TO_RGBQUAD( 0xC0, 0xC0, 0xC0 ),  COLOR_BTNFACE },       // bright gray
    { RGB_TO_RGBQUAD( 0xFF, 0xFF, 0xFF ),  COLOR_BTNHIGHLIGHT }   // white
  };

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif



  BOOL CToolBar::LoadBitmap( LPCTSTR lpszResourceName )
  {
    HINSTANCE   hInstance = GetModuleHandle( NULL );

    // determine location of the bitmap in resource fork
    HRSRC hRsrcImageWell = ::FindResource( hInstance, lpszResourceName, RT_BITMAP );
    if ( hRsrcImageWell == NULL )
      return FALSE;

    // load the bitmap
    HBITMAP hbmImageWell = NULL;

    HGLOBAL hglb;
    if ( ( hglb = LoadResource( hInstance, hRsrcImageWell ) ) == NULL )
      return FALSE;

    LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource( hglb );
    if ( lpBitmap == NULL )
      return FALSE;

    // make copy of BITMAPINFOHEADER so we can modify the color table
    const int nColorTableSize = 16;
    UINT nSize = lpBitmap->biSize + nColorTableSize * sizeof( RGBQUAD );
    LPBITMAPINFOHEADER lpBitmapInfo = ( LPBITMAPINFOHEADER )::malloc( nSize );
    if ( lpBitmapInfo == NULL )
      return FALSE;
    memcpy( lpBitmapInfo, lpBitmap, nSize );

    // color table is in RGBQUAD DIB format
    DWORD* pColorTable =
      (DWORD*)( ( (LPBYTE)lpBitmapInfo ) + (UINT)lpBitmapInfo->biSize );

    for ( int iColor = 0; iColor < nColorTableSize; iColor++ )
    {
      // look for matching RGBQUAD color in original
      for ( int i = 0; i < _countof( _afxSysColorMap ); i++ )
      {
        if ( pColorTable[iColor] == _afxSysColorMap[i].rgbqFrom )
        {
          /*
          if (bMono)
          {
            // all colors except text become white
            if (_afxSysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
              pColorTable[iColor] = RGB_TO_RGBQUAD(255, 255, 255);
          }
          else
          */
          pColorTable[iColor] =
            CLR_TO_RGBQUAD( ::GetSysColor( _afxSysColorMap[i].iSysColorTo ) );
          break;
        }
      }
    }

    int nWidth = (int)lpBitmapInfo->biWidth;
    int nHeight = (int)lpBitmapInfo->biHeight;
    HDC hDCScreen = ::GetDC( NULL );
    hbmImageWell = ::CreateCompatibleBitmap( hDCScreen, nWidth, nHeight );

    if ( hbmImageWell != NULL )
    {
      HDC hDCGlyphs = ::CreateCompatibleDC( hDCScreen );
      HBITMAP hbmOld = ( HBITMAP )::SelectObject( hDCGlyphs, hbmImageWell );

      LPBYTE lpBits;
      lpBits = (LPBYTE)( lpBitmap + 1 );
      lpBits += ( 1 << ( lpBitmapInfo->biBitCount ) ) * sizeof( RGBQUAD );

      StretchDIBits( hDCGlyphs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
                     lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY );
      SelectObject( hDCGlyphs, hbmOld );
      ::DeleteDC( hDCGlyphs );
    }
    ::ReleaseDC( NULL, hDCScreen );

    // free copy of bitmap info struct and resource itself
    ::free( lpBitmapInfo );
    ::FreeResource( hglb );

    // tell common control toolbar about the new bitmap
    if ( !AddReplaceBitmap( hbmImageWell ) )
      return FALSE;

    // remember the resource handles so the bitmap can be recolored if necessary
    //m_hInstImageWell = hInstImageWell;
    //m_hRsrcImageWell = hRsrcImageWell;
    return TRUE;
  }



  /*-AddReplaceBitmap-----------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/

  BOOL CToolBar::AddReplaceBitmap( HBITMAP hbmImageWell )
  {

    // need complete bitmap size to determine number of images

    BITMAP    bitmap;

    ::GetObject( hbmImageWell, sizeof( BITMAP ), &bitmap );


    // add the bitmap to the common control toolbar
    BOOL bResult;
    if ( m_hbmImageWell == NULL )
    {
      TBADDBITMAP     addBitmap;

      addBitmap.hInst = NULL; // makes TBADDBITMAP::nID behave a HBITMAP
      addBitmap.nID = (UINT_PTR)hbmImageWell;

      bResult = SendMessage( TB_ADDBITMAP, bitmap.bmWidth / m_sizeImage.cx, (LPARAM)&addBitmap ) == 0;
    }
    else
    {
      TBREPLACEBITMAP     replaceBitmap;

      replaceBitmap.hInstOld = NULL;
      replaceBitmap.nIDOld = (UINT_PTR)m_hbmImageWell;
      replaceBitmap.hInstNew = NULL;
      replaceBitmap.nIDNew = (UINT_PTR)hbmImageWell;
      replaceBitmap.nButtons = bitmap.bmWidth / m_sizeImage.cx;

      bResult = (BOOL)SendMessage( TB_REPLACEBITMAP, 0, (LPARAM)&replaceBitmap );
    }

    // remove old bitmap, if present
    if ( bResult )
    {
      if ( m_hbmImageWell )
      {
        DeleteObject( m_hbmImageWell );
      }
      m_hbmImageWell = hbmImageWell;
    }

    return bResult;

  }



  BOOL CToolBar::SetButtons( const UINT* lpIDArray, int nIDCount )
  {

    // delete all existing buttons
    int nCount = ( int )::SendMessageW( m_hWnd, TB_BUTTONCOUNT, 0, 0 );
    while ( nCount-- )
    {
      ::SendMessageW( m_hWnd, TB_DELETEBUTTON, 0, 0 );
    }

    ::SendMessageW( m_hWnd, TB_BUTTONSTRUCTSIZE, ( WPARAM )sizeof( TBBUTTON ), 0 );

    TBBUTTON button; memset( &button, 0, sizeof( TBBUTTON ) );
    button.iString = -1;
    if ( lpIDArray != NULL )
    {
      // add new buttons to the common control
      int iImage = 0;
      for ( int i = 0; i < nIDCount; i++ )
      {
        button.fsState = TBSTATE_ENABLED;
        if ( ( button.idCommand = *lpIDArray++ ) == 0 )
        {
          // separator
          button.fsStyle = TBSTYLE_SEP;
          // width of separator includes 8 pixel overlap
          if ( ( GetStyle() & TBSTYLE_FLAT ) )//|| _afxComCtlVersion == VERSION_IE4)
            button.iBitmap = 6;
          else
            button.iBitmap = 8;
        }
        else
        {
          // a command button with image
          button.fsStyle = TBSTYLE_BUTTON;
          button.iBitmap = iImage++;
        }
        if ( !::SendMessageW( m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&button ) )
        {
          return FALSE;
        }
      }
    }
    else
    {
      // add 'blank' buttons
      button.fsState = TBSTATE_ENABLED;
      for ( int i = 0; i < nIDCount; i++ )
      {
        if ( !::SendMessageW( m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&button ) )
        {
          return FALSE;
        }
      }
    }
    //m_nCount = (int)DefWindowProc(TB_BUTTONCOUNT, 0, 0);
    //m_bDelayedButtonLayout = TRUE;

    return TRUE;
  }



  void CToolBar::EnableButton( DWORD dwButtonId, BOOL bEnable )
  {
    SendMessage( TB_ENABLEBUTTON, dwButtonId, MAKELONG( bEnable, 0 ) );
  }


}