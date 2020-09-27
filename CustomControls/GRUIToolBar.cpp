#include ".\gruitoolbar.h"
#include "GRUIDockFrame.h"



#pragma comment( lib, "comctl32.lib" )

GRUIToolBar::GRUIToolBar()
{

  m_iHighlightedButton    = -1;
  m_iPushedButton         = -1;

  m_dwTimerID             = 0;

  m_hImageList            = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 0, 4 );

}

GRUIToolBar::~GRUIToolBar()
{
  if ( m_hImageList )
  {
    ImageList_Destroy( m_hImageList );
    m_hImageList = NULL;
  }

}



/*
BOOL GRUIToolBar::CreateEx( CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, CRect rcBorders, UINT nID, const char* szTitle )
{

  return CCustomWnd::Create( szTitle, dwCtrlStyle, pParentWnd, nID );

}
*/


BOOL GRUIToolBar::LoadToolBar( HINSTANCE hInst, DWORD dwResourceID )
{

  return LoadToolBar( hInst, MAKEINTRESOURCE( dwResourceID ) );

}



struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items()
		{ return (WORD*)(this+1); }
};


BOOL GRUIToolBar::LoadToolBar( HINSTANCE hInst, LPCTSTR lpszResourceName)
{

	// determine location of the bitmap in resource fork
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, MAKEINTRESOURCE(241));
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	//ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
  {
		pItems[i] = pData->items()[i];

    AddButton( pItems[i] );
  }
	BOOL bResult = TRUE;//SetButtons(pItems, pData->wItemCount);
	delete[] pItems;

	if ( bResult )
	{
		// load bitmap now that sizes are known by the toolbar control
		bResult = LoadBitmap( hInst, lpszResourceName );
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

  CalcSize();

	return bResult;
}



BOOL GRUIToolBar::LoadBitmap( HINSTANCE hInstImageWell, LPCTSTR lpszResourceName )
{
	HRSRC hRsrcImageWell = ::FindResource(hInstImageWell, lpszResourceName, RT_BITMAP);
	if ( hRsrcImageWell == NULL )
  {
		return FALSE;
  }

	// load the bitmap
  if ( m_hImageList )
  {
    ImageList_Destroy( m_hImageList );
    m_hImageList = NULL;
  }

  m_hImageList = ImageList_LoadImage( hInstImageWell, lpszResourceName, 16, 2, GetSysColor( COLOR_BTNFACE ), 
                                      IMAGE_BITMAP, 
                                      LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

	return TRUE;

}



BOOL GRUIToolBar::AddButton( DWORD dwCommandID )
{

  tToolBarButtonInfo   BtnInfo;

  BtnInfo.dwCommandID = dwCommandID;
  SetRect( &BtnInfo.rcButton, 0, 0, 23, 22 );

  if ( dwCommandID == 0 )
  {
    // ein Separator
    BtnInfo.rcButton.right = 4;
  }

  m_vectButtons.push_back( BtnInfo );

  CalcSize();

  return TRUE;

}



SIZE GRUIToolBar::EstimateSize( DWORD dwSizingFlags )
{

  bool      bEstimateHorizontal = true;

  if ( ( dwSizingFlags & GRCBS_DOCKED_LEFT )
  ||   ( dwSizingFlags & GRCBS_DOCKED_RIGHT ) )
  {
    bEstimateHorizontal = false;
  }

  SIZE      szCB;

  szCB.cx = 0;
  szCB.cy = 0;

  int       iX = 0,
            iY = 0;

  if ( !IsFloating() )
  {
    // angedockt kommt ein Gripper dran
    if ( bEstimateHorizontal )
    {
      szCB.cx += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperWidth;
      iX += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperWidth;
    }
    else
    {
      szCB.cy += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperHeight;
      iY += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperHeight;
    }
  }

  tVectButtons    vectTemp = m_vectButtons;


  for ( size_t i = 0; i < vectTemp.size(); ++i )
  {
    tToolBarButtonInfo&    BtnInfo = vectTemp[i];

    BtnInfo.rcButton.left   = iX;
    BtnInfo.rcButton.top    = iY;
    if ( !bEstimateHorizontal )
    {
      BtnInfo.rcButton.right  = BtnInfo.rcButton.left + 23;
      if ( BtnInfo.dwCommandID == 0 )
      {
        // ein Separator
        szCB.cy += 4;

        BtnInfo.rcButton.bottom = BtnInfo.rcButton.top + 4;
      }
      else
      {
        BtnInfo.rcButton.bottom = BtnInfo.rcButton.top + 22;

        szCB.cy += 22;
        if ( 22 > szCB.cx )
        {
          szCB.cx += 22;
        }
      }
      iY += BtnInfo.rcButton.bottom - BtnInfo.rcButton.top;
    }
    else
    {
      BtnInfo.rcButton.bottom = BtnInfo.rcButton.top + 22;
      if ( BtnInfo.dwCommandID == 0 )
      {
        // ein Separator
        szCB.cx += 4;

        BtnInfo.rcButton.right = BtnInfo.rcButton.left + 4;
      }
      else
      {
        BtnInfo.rcButton.right = BtnInfo.rcButton.left + 23;
        szCB.cx += 23;
        if ( 22 > szCB.cy )
        {
          szCB.cy += 22;
        }
      }
      iX += BtnInfo.rcButton.right - BtnInfo.rcButton.left;
    }
  }

  if ( !IsFloating() )
  {
    // angedockt kommt ein Gripper dran
    if ( !bEstimateHorizontal )
    {
      szCB.cx++;
    }
  }

  if ( dwSizingFlags & GRCBS_APPLY_SIZE )
  {
    m_bHorizontal = bEstimateHorizontal;
    m_vectButtons = vectTemp;
  }

  return szCB;

}



void GRUIToolBar::Draw( HDC hdc, const RECT& rc )
{

  m_pDisplayClass->PaintToolBarBack( hdc, rc, m_bHorizontal );

  int   iButtonIndex = 0,
        iX = 0,
        iY = 0;

  // Gripper zeichnen
  RECT      rcGripper;

  if ( GetGripperRect( rcGripper ) )
  {
    if ( m_bHorizontal )
    {
      iX += rcGripper.right - rcGripper.left;
    }
    else
    {
      iY += rcGripper.bottom - rcGripper.top;
    }
    m_pDisplayClass->PaintToolBarGripper( hdc, rcGripper, m_bHorizontal );
  }

  for ( size_t i = 0; i < m_vectButtons.size(); ++i )
  {
    tToolBarButtonInfo&    BtnInfo = m_vectButtons[i];

    int   iCX = ( BtnInfo.rcButton.right - BtnInfo.rcButton.left - 16 ) / 2;
    int   iCY = ( BtnInfo.rcButton.bottom - BtnInfo.rcButton.top - 15 ) / 2;

    if ( ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_PUSHED )
    ||   ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_CHECKED ) )
    {
      iCX++;
      iCY++;
    }

    m_pDisplayClass->PaintToolBarButton( hdc, iX, iY, BtnInfo, m_hImageList, iButtonIndex );
    if ( m_bHorizontal )
    {
      iX += BtnInfo.rcButton.right - BtnInfo.rcButton.left;
    }
    else
    {
      iY += BtnInfo.rcButton.bottom - BtnInfo.rcButton.top;
    }
    ++iButtonIndex;
  }

  CCustomWnd::Draw( hdc, rc );

}



void GRUIToolBar::LButtonDown(UINT nFlags, POINT point)
{

  RECT    rcGripper;

  if ( GetGripperRect( rcGripper ) )
  {
    if ( PtInRect( &rcGripper, point ) )
    {
      ClientToScreen( &point );
      m_pFrameWnd->StartDragging( point );
      return;
    }
  }

  size_t   iOverButton = -1;

  for ( size_t i = 0; i < m_vectButtons.size(); ++i )
  {
    tToolBarButtonInfo&    BtnInfo = m_vectButtons[i];

    if ( ( BtnInfo.dwCommandID )
    &&   ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED ) ) )
    {
      if ( PtInRect( &BtnInfo.rcButton, point ) )
      {
        iOverButton = i;
        break;
      }
    }
  }

  if ( iOverButton != m_iHighlightedButton )
  {
    if ( m_iHighlightedButton != -1 )
    {
      InvalidateRect( &m_vectButtons[m_iHighlightedButton].rcButton );
      m_vectButtons[m_iHighlightedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_HILIGHTED;
      m_iHighlightedButton = -1;
    }
    m_iHighlightedButton  = iOverButton;
  }
  if ( m_iPushedButton != iOverButton )
  {
    m_iPushedButton       = iOverButton;
    if ( iOverButton != -1 )
    {
      m_vectButtons[iOverButton].dwButtonStyle |= tToolBarButtonInfo::BTN_HILIGHTED | tToolBarButtonInfo::BTN_PUSHED;
      InvalidateRect( &m_vectButtons[iOverButton].rcButton );
      SetCapture();
    }
  }

}

void GRUIToolBar::LButtonUp(UINT nFlags, POINT point)
{

  if ( GetCapture() == GetSafeHwnd() )
  {
    if ( ( m_iHighlightedButton == m_iPushedButton )
    &&   ( m_iPushedButton != -1 ) )
    {
      //GetTopLevelFrame()->PostMessage( WM_COMMAND, MAKEWPARAM( m_vectButtons[m_iPushedButton].dwCommandID, 0 ), (LPARAM)GetSafeHwnd() );
    }
    ReleaseCapture();
  }
  if ( m_iHighlightedButton != -1 )
  {
    InvalidateRect( &m_vectButtons[m_iHighlightedButton].rcButton );
    m_vectButtons[m_iHighlightedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_HILIGHTED;
    m_iHighlightedButton = -1;
  }
  if ( m_iPushedButton != -1 )
  {
    InvalidateRect( &m_vectButtons[m_iPushedButton].rcButton );
    m_vectButtons[m_iPushedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_PUSHED;
    m_iPushedButton = -1;
  }

}



void GRUIToolBar::MouseMove(UINT nFlags, POINT point)
{

  size_t   iOverButton = -1;

  for ( size_t i = 0; i < m_vectButtons.size(); ++i )
  {
    tToolBarButtonInfo&    BtnInfo = m_vectButtons[i];

    if ( ( BtnInfo.dwCommandID )
    &&   ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED ) ) )
    {
      if ( PtInRect( &BtnInfo.rcButton, point ) )
      {
        iOverButton = i;
        break;
      }
    }
  }

  if ( iOverButton != m_iHighlightedButton )
  {
    if ( m_iHighlightedButton != -1 )
    {
      InvalidateRect( &m_vectButtons[m_iHighlightedButton].rcButton );
      m_vectButtons[m_iHighlightedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_HILIGHTED;
      m_iHighlightedButton = -1;
    }
    if ( m_iPushedButton != -1 )
    {
      if ( m_vectButtons[m_iPushedButton].dwButtonStyle & tToolBarButtonInfo::BTN_PUSHED )
      {
        m_vectButtons[m_iPushedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_PUSHED;
        InvalidateRect( &m_vectButtons[m_iPushedButton].rcButton );
      }
    }
    m_iHighlightedButton  = iOverButton;
    if ( iOverButton != -1 )
    {
      if ( m_iHighlightedButton == m_iPushedButton )
      {
        m_vectButtons[m_iHighlightedButton].dwButtonStyle |= tToolBarButtonInfo::BTN_PUSHED;
      }
      if ( m_iPushedButton == -1 )
      {
        m_vectButtons[m_iHighlightedButton].dwButtonStyle |= tToolBarButtonInfo::BTN_HILIGHTED;
      }
      InvalidateRect( &m_vectButtons[iOverButton].rcButton );
      if ( m_dwTimerID == 0 )
      {
        m_dwTimerID = (DWORD)SetTimer( 7, 50, NULL );
      }
    }
  }

  if ( ( m_dwTimerID != 0 )
  &&   ( m_iHighlightedButton == -1 ) )
  {
    KillTimer( m_dwTimerID );
    m_dwTimerID = 0;
  }

  if ( m_iHighlightedButton != -1 )
  {
    // Statusbar-Text
    /*
    AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
    if ( ( pModuleThreadState->m_pLastStatus != (CControlBar*)this )
    ||   ( pModuleThreadState->m_nLastStatus != m_vectButtons[m_iHighlightedButton].dwCommandID ) )
    {
  	  pModuleThreadState->m_pLastStatus = (CControlBar*)this;
	    pModuleThreadState->m_nLastStatus = m_vectButtons[m_iHighlightedButton].dwCommandID;
      GetOwner()->SendMessage(WM_SETMESSAGESTRING, m_vectButtons[m_iHighlightedButton].dwCommandID);
    }
    */
  }
  else
  {
    /*
    AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
    if ( pModuleThreadState->m_pLastStatus == (CControlBar*)this )
    {
  	  pModuleThreadState->m_pLastStatus = NULL;
	    pModuleThreadState->m_nLastStatus = -1;
      GetOwner()->SendMessage( WM_POPMESSAGESTRING, AFX_IDS_IDLEMESSAGE );
    }
    */
  }

}



void GRUIToolBar::OnDestroy()
{

  if ( m_dwTimerID != 0 )
  {
    KillTimer( m_dwTimerID );
    m_dwTimerID = 0;
  }

}



void GRUIToolBar::OnTimer( UINT_PTR nIDEvent)
{

  POINT   ptMouse;

  GetCursorPos( &ptMouse );

  ScreenToClient( &ptMouse );

  RECT    rc;

  GetClientRect( &rc );

  if ( !PtInRect( &rc, ptMouse ) )
  {
    if ( m_iHighlightedButton != -1 )
    {
      InvalidateRect( &m_vectButtons[m_iHighlightedButton].rcButton );
      m_vectButtons[m_iHighlightedButton].dwButtonStyle &= ~tToolBarButtonInfo::BTN_HILIGHTED;
      m_iHighlightedButton = -1;

      /*
      AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
      if ( pModuleThreadState->m_pLastStatus == (CControlBar*)this )
      {
  	    pModuleThreadState->m_pLastStatus = NULL;
	      pModuleThreadState->m_nLastStatus = -1;
        GetOwner()->SendMessage( WM_POPMESSAGESTRING, AFX_IDS_IDLEMESSAGE );
      }
      */
    }
    KillTimer( m_dwTimerID );
    m_dwTimerID = 0;
  }

}



void GRUIToolBar::EnableControl( UINT nIndex, BOOL bOn )
{

  if ( nIndex >= m_vectButtons.size() )
  {
    return;
  }

  if ( ( ( bOn )
  &&     ( !( m_vectButtons[nIndex].dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED ) ) )
  ||   ( ( !bOn )
  &&     ( m_vectButtons[nIndex].dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED ) ) )
  {
    // nichts zu ändern
    return;
  }

  if ( bOn )
  {
    m_vectButtons[nIndex].dwButtonStyle &= ~tToolBarButtonInfo::BTN_DISABLED;
  }
  else
  {
    m_vectButtons[nIndex].dwButtonStyle |= tToolBarButtonInfo::BTN_DISABLED;
  }

  InvalidateRect( &m_vectButtons[nIndex].rcButton );

}


void GRUIToolBar::SetCheck( UINT nIndex, BOOL bSet )
{

  if ( nIndex >= m_vectButtons.size() )
  {
    return;
  }

  tToolBarButtonInfo&    BtnInfo = m_vectButtons[nIndex];

  if ( ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_CHECKABLE ) )
  &&   ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_RADIO ) ) )
  {
    return;
  }

  if ( ( ( bSet )
  &&     ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_CHECKED ) )
  ||   ( ( !bSet )
  &&     ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_CHECKED ) ) ) )
  {
    // nichts zu ändern
    return;
  }

  if ( bSet )
  {
    BtnInfo.dwButtonStyle |= tToolBarButtonInfo::BTN_CHECKED;
  }
  else
  {
    BtnInfo.dwButtonStyle &= ~tToolBarButtonInfo::BTN_CHECKED;
  }

  InvalidateRect( &m_vectButtons[nIndex].rcButton );

  if ( ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_RADIO )
  &&   ( bSet ) )
  {
    // alle Radios links und rechts (in meiner Gruppe also) entradioen
    UINT    nTempIndex = nIndex;
    while ( nTempIndex )
    {
      nTempIndex--;
      if ( ( m_vectButtons[nTempIndex].dwCommandID )
      &&   ( m_vectButtons[nTempIndex].dwButtonStyle & tToolBarButtonInfo::BTN_RADIO ) )
      {
        SetCheck( nTempIndex, FALSE );
      }
    }
    nTempIndex = nIndex;
    while ( nTempIndex + 1 < m_vectButtons.size() )
    {
      nTempIndex++;
      if ( ( m_vectButtons[nTempIndex].dwCommandID )
      &&   ( m_vectButtons[nTempIndex].dwButtonStyle & tToolBarButtonInfo::BTN_RADIO ) )
      {
        SetCheck( nTempIndex, FALSE );
      }
    }
  }

}



BOOL GRUIToolBar::AddButton( DWORD dwCommandID, HICON hIcon )
{

  if ( hIcon == NULL )
  {
    return FALSE;
  }

  if ( !AddButton( dwCommandID ) )
  {
    return FALSE;
  }

  ImageList_AddIcon( m_hImageList, hIcon );

  CalcSize();

  return TRUE;

}



BOOL GRUIToolBar::AddCheckButton( DWORD dwCommandID, HICON hIcon )
{

  if ( !AddButton( dwCommandID, hIcon ) )
  {
    return FALSE;
  }

  tToolBarButtonInfo&    BtnInfo = m_vectButtons.back();

  BtnInfo.dwButtonStyle |= tToolBarButtonInfo::BTN_CHECKABLE;

  return TRUE;

}



BOOL GRUIToolBar::AddRadioButton( DWORD dwCommandID, HICON hIcon )
{

  if ( !AddButton( dwCommandID, hIcon ) )
  {
    return FALSE;
  }

  tToolBarButtonInfo&    BtnInfo = m_vectButtons.back();

  BtnInfo.dwButtonStyle |= tToolBarButtonInfo::BTN_RADIO;

  return TRUE;

}



BOOL GRUIToolBar::AddSeparator()
{

  return AddButton( 0 );

}



INT_PTR GRUIToolBar::OnToolHitTest( POINT point, TOOLINFO* pTI ) const
{

	// now hit test against CToolBar buttons
  for ( size_t i = 0; i < m_vectButtons.size(); ++i )
  {
    const tToolBarButtonInfo&    BtnInfo = m_vectButtons[i];

    if ( ( BtnInfo.dwCommandID )
    &&   ( !( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED ) ) )
    {
      if ( PtInRect( &BtnInfo.rcButton, point ) )
      {
				UINT_PTR uHit = BtnInfo.dwCommandID;
        typedef struct tagAFX_OLDTOOLINFO {
	        UINT cbSize;
	        UINT uFlags;
	        HWND hwnd;
	        UINT uId;
	        RECT rect;
	        HINSTANCE hinst;
	        LPTSTR lpszText;
        } AFX_OLDTOOLINFO;

				if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
				{
					pTI->hwnd = m_hWnd;
					pTI->rect = BtnInfo.rcButton;
					pTI->uId = uHit;
					pTI->lpszText = LPSTR_TEXTCALLBACK;
				}
				// found matching rect, return the ID of the button
				return uHit != 0 ? static_cast<INT_PTR>(uHit) : static_cast<INT_PTR>(-1);
      }
    }
  }

  return -1;

}



LRESULT GRUIToolBar::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_CAPTURECHANGED:
      if ( (HWND)lParam != GetSafeHwnd() )
      {
      }
      break;
    case WM_CREATE:
      CalcSize();
      break;
    case WM_LBUTTONDOWN:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        LButtonDown( (UINT)wParam, pt );
      }
      break;
    case WM_LBUTTONUP:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        LButtonUp( (UINT)wParam, pt );
      }
      break;
    case WM_MOUSEMOVE:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        MouseMove( (UINT)wParam, pt );
      }
      break;
    case WM_DESTROY:
      OnDestroy();
      break;
    case WM_TIMER:
      OnTimer( (UINT_PTR)wParam );
      break;
  }

  return GRUIControlBar::WindowProc( uMsg, wParam, lParam );

}