#include "ScriptEditCtrl.h"


BOOL CScriptEditCtrl::m_bRegistered = Register();		// Register the control during class initialization



BOOL CScriptEditCtrl::Register()
{
	// Register the window class of the control
	WNDCLASS	wc;
  wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;	// Usual style bits
  wc.lpfnWndProc = ::DefWindowProc;					// Message processing code
	wc.cbClsExtra = 0;								// No extra bytes needed
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;							// No instance handle
	wc.hIcon = NULL;								// No icon
	wc.hCursor = ::LoadCursor(NULL, IDC_IBEAM);		// Use I-beam cursor (like edit control)
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);	// Use default window color (overriden in OnEraseBkgnd)
	wc.lpszMenuName = NULL;							// No menus
	wc.lpszClassName = _T( "GRScriptEdit" );				// Class name
	if (!::RegisterClass(&wc))						// If registration failed, subsequent dialogs will fail
	{
		return FALSE;
	}
	else
		return TRUE;
}


CScriptEditCtrl::CScriptEditCtrl()
{
  m_hFont = CreateFont( 16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 
                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      DEFAULT_PITCH, _T( "Courier New" ) );

  m_bFocus                  = FALSE;
  m_bSelection              = FALSE;
  m_bSelectingByMouse       = FALSE;

  m_bModified               = FALSE;

  m_iCharWidth              = 9;
  m_iCharHeight             = 17;
  m_iVisibleCharsPerLine    = 0;
  m_iVisibleLines           = 0;

  m_dwTabSpacing            = 2;
  m_dwFlags                 = SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS;

  m_vectColor.push_back( GetSysColor( COLOR_WINDOW ) );
  m_vectColor.push_back( GetSysColor( COLOR_WINDOWTEXT ) );
  m_vectColor.push_back( GetSysColor( COLOR_HIGHLIGHT ) );
  m_vectColor.push_back( GetSysColor( COLOR_HIGHLIGHTTEXT ) );
  m_vectColor.push_back( GetSysColor( COLOR_INACTIVEBORDER ) );
  m_vectColor.push_back( GetSysColor( COLOR_GRAYTEXT ) );
  m_vectColor.push_back( RGB( 150, 50, 50 ) );
  m_vectColor.push_back( RGB( 50, 150, 50 ) );

  m_bEnabled                = TRUE;

  Reset();

  // Keyword-Liste löschen
  m_vectSyntaxEntry.clear();
}



CScriptEditCtrl::~CScriptEditCtrl()
{
  DeleteObject( m_hFont );
}



BOOL CScriptEditCtrl::Create( int iX, int iY, int iWidth, int iHeight, HWND hwndParent, int iID, DWORD dwFlags )
{

  RECT      rc;

  rc.left = iX;
  rc.top = iY;
  rc.right = iX + iWidth;
  rc.bottom = iY + iHeight;

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, _T( "GRScriptEdit" ), _T( "" ),
                        WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, CWnd::FromHandle( hwndParent ), iID ) )
  {
    return FALSE;
  }
  SetScrollRange( SB_HORZ, 0, SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH );
  SetScrollRange( SB_VERT, 0, 0 );

  GetClientRect( &m_rectRC );
  m_iVisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_iCharWidth;
  m_iVisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_iCharHeight;

  CreateSolidCaret( 1, (int)m_iCharHeight );
  m_bShowCaret = FALSE;

  m_dwFlags = dwFlags;

  SetTimer( 1, 100, NULL );

  EnableWindow();

  return TRUE;
}



void CScriptEditCtrl::Reset()
{
  m_iFirstLine              = 0;
  m_iFirstCharacter         = 0;
  m_iCursorWantedX          = -1;
  m_iCursorX                = 0;
  m_iCursorY                = 0;
  m_iCaretX                 = 0;

  m_iSelectionAnchorX       = 0;
  m_iSelectionAnchorY       = 0;

  // Text-Vektor initialisieren (sonst gibt's HaufenvieleErrors)
  m_vectText.clear();
  m_vectText.push_back( "" );

  m_bModified               = FALSE;

  if ( m_hWnd )
  {
    AdjustScrollBars();
    Invalidate();
  }
}



BOOL CScriptEditCtrl::SetWindowPos( HWND hwndAfter, int x, int y, int cx, int cy, UINT nFlags )
{

  if ( !::IsWindow( m_hWnd ) )
  {
    return FALSE;
  }
  BOOL bResult = SetWindowPos( hwndAfter, x, y, cx, cy, nFlags );

  if ( !( nFlags & SWP_NOSIZE ) )
  {
    AdjustScrollBars();
    ScrollIntoView();
    Invalidate();
  }
  return bResult;

}



BOOL CScriptEditCtrl::LoadFromFile( const char *szFileName )
{
  Reset();

  HANDLE        hFile;

  hFile = CreateFileA( szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  if ( hFile == INVALID_HANDLE_VALUE )
  {
    return FALSE;
  }
  do
  {
    char            a         = '\0';

    unsigned long   ulOffset  = 0,
                    ulBytesRead;
    GR::String     strDummy  = "";

    while ( TRUE )
    {
      ReadFile( hFile, &a, 1, &ulBytesRead, NULL );
      if ( ulBytesRead == 0 )
      {
        m_vectText.push_back( strDummy );
        CloseHandle( hFile );
        hFile = INVALID_HANDLE_VALUE;
        break;
      }
      if ( ( ulBytesRead == 0 )
      ||   ( a == 10 ) )
      {
        m_vectText.push_back( strDummy );
        break;
      }
      else if ( a != 13 )
      {
        if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
        {
          if ( a == VK_TAB )
          {
            for ( DWORD i = 0; i < m_dwTabSpacing; i++ )
            {
              strDummy += ' ';
            }
            continue;
          }
        }
        strDummy += a;
      }
    }
  }
  while ( hFile != INVALID_HANDLE_VALUE );
  
  m_bModified = FALSE;
  AdjustScrollBars();
  ScrollIntoView();
  UpdateSelectionAnchor( TRUE );
  Invalidate();
  return TRUE;

}



void CScriptEditCtrl::SaveToFile( const char *szFileName )
{
  HANDLE        hFile;


  hFile  = CreateFileA( szFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  if ( hFile == INVALID_HANDLE_VALUE )
  {
    return;
  }
  DWORD     dwBytesWritten;

  char      caNL[2] = { 13, 10 };

  for ( size_t i = 0; i < m_vectText.size(); i++ )
  {
    // Zeile
    WriteFile( hFile, m_vectText[i].c_str(), (DWORD)m_vectText[i].length(), &dwBytesWritten, NULL );
    // #13#10
    WriteFile( hFile, caNL, 2, &dwBytesWritten, NULL );
  }

  CloseHandle( hFile );
}



GR::String CScriptEditCtrl::GetLine( DWORD dwLine )
{
  GR::String     strResult = "";

  if ( dwLine < m_vectText.size() )
  {
    strResult = m_vectText[dwLine];
  }
  return strResult;
}



DWORD CScriptEditCtrl::GetLineCount()
{
  return (DWORD)m_vectText.size();
}



void CScriptEditCtrl::GetWindowText( GR::String& strText ) const
{

  strText = "";
  for ( size_t i = 0; i < m_vectText.size(); i++ )
  {
    strText += m_vectText[i].c_str();
    strText += "\r\n";
  }
  

}



void CScriptEditCtrl::OnPaint() 
{

  if ( m_vectText.size() == 0 )
  {
    return;
  }
  DrawCursor();


  HDC   hdcOld;

	PAINTSTRUCT ps;
	
  hdcOld = ::BeginPaint( m_hWnd, &ps );

  // flimmerfreies Zeichnen
  HDC     hdc = CreateCompatibleDC( hdcOld );

  RECT      rectMem;

  HBITMAP   m_bitmapMem;
  HBITMAP   m_oldBitmap;

  GetClipBox( hdcOld, &rectMem );
  m_bitmapMem = CreateCompatibleBitmap( hdcOld, rectMem.right - rectMem.left, rectMem.bottom - rectMem.top );
	m_oldBitmap = (HBITMAP)SelectObject( hdc, m_bitmapMem );

	SetWindowOrgEx( hdc, rectMem.left, rectMem.top, NULL );


	HFONT hOldFont = (HFONT)SelectObject( hdc, m_hFont );
	SetBkMode( hdc, TRANSPARENT );
	size_t x;
	size_t y = 0;
  GR::String     temp;

  GetClientRect( &m_rectRC );
  m_iVisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_iCharWidth;
  m_iVisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_iCharHeight;
  if ( m_bEnabled )
  {
    FillSolidRect( hdc, m_rectRC, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
  }
  else
  {
    FillSolidRect( hdc, m_rectRC, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
  }
  for ( size_t p = m_iFirstLine; p < m_iFirstLine + m_iVisibleLines + 1; p++ )
	{
    if ( p >= m_vectText.size() )
    {
      break;
    }
    if ( m_vectText[p].length() == 0 )
    {
      y += m_iCharHeight;
      continue;
    }

    std::vector<COLORREF>   vectColors;

    vectColors.resize( m_vectText[p].length() + 2 );
    FillColorVector( m_vectText[p], p, &vectColors );

    x = 0;
		temp = m_vectText[p];

    if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
    {
		  for ( size_t j = m_iFirstCharacter; j < temp.length(); j++ )
		  {
        if ( IsInsideSelection( j, p ) )
        {
          FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
          SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
        }
        else
        {
          if ( m_bEnabled )
          {
            SetTextColor( hdc, vectColors[j] );
            FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
          }
          else
          {
            SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
            FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
          }
        }
        TextOutA( hdc, (int)x, (int)y, temp.substr( j, 1 ).c_str(), 1 );
        x += m_iCharWidth;
		  }
    }
    else
    {
      size_t     iRealX,
              iLeftSpaces,
              iPos;
      iRealX = 0;
      iLeftSpaces = 0;

      iPos = 0;

      do
		  {
        if ( iRealX > m_iFirstCharacter + m_iVisibleCharsPerLine )
        {
          break;
        }
        if ( iLeftSpaces )
        {
          iLeftSpaces--;
          if ( iRealX >= m_iFirstCharacter )
          {
            if ( IsInsideSelection( iPos, p ) )
            {
              FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
              SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
            }
            else
            {
              if ( m_bEnabled )
              {
                SetTextColor( hdc, vectColors[iPos] );
                FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
              }
              else
              {
                SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
                FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
              }
            }

			      TextOutA( hdc, (int)x, (int)y, " ", 1 );
            x += m_iCharWidth;
          }
          iRealX++;
          if ( iLeftSpaces == 0 )
          {
            iPos++;
          }
        }
        else
        {
          if ( temp[iPos] == VK_TAB )
          {
            iLeftSpaces = m_dwTabSpacing;
          }
          else
          {
            if ( iRealX >= m_iFirstCharacter )
            {
              if ( IsInsideSelection( iPos, p ) )
              {
                FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
                SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
              }
              else
              {
                if ( m_bEnabled )
                {
                  SetTextColor( hdc, vectColors[iPos] );
                  FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
                }
                else
                {
                  SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
                  FillSolidRect( hdc, (int)x, (int)y, (int)m_iCharWidth, (int)m_iCharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
                }
              }

              TextOutA( hdc, (int)x, (int)y, temp.substr( iPos, 1 ).c_str(), 1 );
              x += m_iCharWidth;
            }
            iPos++;
            iRealX++;
          }
        }
		  }
      while ( iPos < (int)temp.length() );
    }
		y += m_iCharHeight;
	}

  // Doppelpuffer zurückkopieren
  BitBlt( hdcOld, rectMem.left, rectMem.top, rectMem.right - rectMem.left, rectMem.bottom - rectMem.top,
		              hdc, rectMem.left, rectMem.top, SRCCOPY );
	SelectObject( hdc, m_oldBitmap );
  DeleteObject( m_bitmapMem );

  SelectObject( hdc, hOldFont );

  DeleteDC( hdc );

  ::EndPaint( m_hWnd, &ps );
}



BOOL CScriptEditCtrl::IsAbove( size_t iX1, size_t iY1, size_t iX2, size_t iY2 )
{
  if ( iY1 < iY2 )
  {
    return TRUE;
  }
  if ( iY1 > iY2 )
  {
    return FALSE;
  }
  // Spezialfall, beide auf der selben Höhe
  if ( iX1 <= iX2 )
  {
    return TRUE;
  }
  return FALSE;

}



BOOL CScriptEditCtrl::IsInsideSelection( size_t iX, size_t iY )
{

  if ( !m_bSelection )
  {
    return FALSE;
  }
  size_t     iSelectionStartX    = m_iSelectionAnchorX,
          iSelectionStartY    = m_iSelectionAnchorY,
          iSelectionEndX      = m_iCursorX,
          iSelectionEndY      = m_iCursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_iCursorX;
    iSelectionStartY    = m_iCursorY;
    iSelectionEndX      = m_iSelectionAnchorX;
    iSelectionEndY      = m_iSelectionAnchorY;
  }
  if ( ( iSelectionStartY > iY )
  ||   ( iSelectionEndY < iY ) )
  {
    return FALSE;
  }
  BOOL      bNachBeginn = FALSE,
            bVorEnde    = FALSE;

  if ( iSelectionStartY < iY )
  {
    bNachBeginn = TRUE;
  }
  else
  {
    // der Selection-Start ist auf der selben Zeile
    if ( iSelectionStartX <= iX )
    {
      // die Selection beginnt vor unserer Position
      bNachBeginn = TRUE;
    }
    else
    {
      bNachBeginn = FALSE;
    }
  }
  if ( iSelectionEndY > iY )
  {
    bVorEnde = TRUE;
  }
  else
  {
    // das Selection-Ende ist auf der selben Zeile
    if ( iSelectionEndX > iX )
    {
      // die Selection beginnt vor unserer Position
      bVorEnde = TRUE;
    }
    else
    {
      bVorEnde = FALSE;
    }
  }
  if ( ( bNachBeginn )
  &&   ( bVorEnde ) )
  {
    return TRUE;
  }
  return FALSE;

}



std::vector<GR::String>::iterator CScriptEditCtrl::GetStringIteratorAt( size_t iY )
{

  if ( ( iY < 0 )
  ||   ( m_vectText.size() == 0 )
  ||   ( iY >= m_vectText.size() ) )
  {
    return m_vectText.end();
  }

  std::vector<GR::String>::iterator      itDummy = m_vectText.begin();
  size_t   iPos = 0;
  while ( iPos < iY )
  {
    iPos++;
    itDummy++;
  }
  return itDummy;

}



DWORD CScriptEditCtrl::GetMaxLength()
{

  if ( m_vectText.size() == 0 )
  {
    return 0;
  }
  DWORD     dwLength = 0,
            dwTrueLength = 0;
  for ( size_t i = 0; i < m_vectText.size(); i++ )
  {
    dwTrueLength = GetTrueLength( m_vectText[i] );
    if ( dwTrueLength > dwLength )
    {
      dwLength = dwTrueLength;
    }
  }
  return dwLength;

}



DWORD CScriptEditCtrl::GetTrueLength( GR::String strLine )
{

  if ( m_vectText.size() == 0 )
  {
    return 0;
  }
  if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return (DWORD)strLine.length();
  }
  DWORD     dwPos = 0,
            dwTrueLength = 0;
  do
  {
    if ( strLine[dwPos] == VK_TAB )
    {
      dwTrueLength += m_dwTabSpacing;
    }
    else
    {
      dwTrueLength++;
    }
    dwPos++;
  }
  while ( dwPos < strLine.length() );

  return dwTrueLength;

}



DWORD CScriptEditCtrl::GetTextPosition( int iTrueX, int iY )
{

  if ( m_vectText.size() == 0 )
  {
    return 0;
  }
  if ( iTrueX == 0 )
  {
    return 0;
  }
  if ( iY >= (int)m_vectText.size() )
  {
    return 0;
  }
  if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return iTrueX;
  }
  int       iDummy = 0;
  DWORD     dwPos = 0;
  while ( iDummy < iTrueX )
  {
    if ( m_vectText[iY][dwPos] == VK_TAB )
    {
      iDummy += m_dwTabSpacing;
    }
    else
    {
      iDummy++;
    }
    dwPos++;
  }
  return dwPos;

}



DWORD CScriptEditCtrl::GetVisiblePosition( int iX, int iY )
{

  if ( m_vectText.size() == 0 )
  {
    return 0;
  }
  if ( iX == 0 )
  {
    return 0;
  }
  if ( iY >= (int)m_vectText.size() )
  {
    return 0;
  }
  if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return iX;
  }
  int       iDummy = 0;
  DWORD     dwPos = 0;
  while ( iDummy < iX )
  {
    if ( m_vectText[iY][dwPos] == VK_TAB )
    {
      dwPos += m_dwTabSpacing;
    }
    else
    {
      dwPos++;
    }
    iDummy++;
  }
  return dwPos;

}



void CScriptEditCtrl::ReplaceSelection( GR::String& strNew, BOOL bSelect /* = TRUE */ )
{

  size_t     iSelectionStartX    = m_iSelectionAnchorX,
          iSelectionStartY    = m_iSelectionAnchorY,
          iSelectionEndX      = m_iCursorX,
          iSelectionEndY      = m_iCursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_iCursorX;
    iSelectionStartY    = m_iCursorY;
    iSelectionEndX      = m_iSelectionAnchorX;
    iSelectionEndY      = m_iSelectionAnchorY;
  }

  size_t   iAnchorX = m_iSelectionAnchorX,
            iAnchorY = m_iSelectionAnchorY;

  RemoveSelection();
  InsertString( strNew, TRUE, bSelect );

  if ( bSelect )
  {
    m_bSelection = TRUE;
    m_iSelectionAnchorX = iSelectionStartX;
    m_iSelectionAnchorY = iSelectionStartY;
  }
  else
  {
    m_bSelection = FALSE;

  }
  m_bModified = TRUE;

}



void CScriptEditCtrl::RemoveSelection()
{

  if ( !m_bEnabled )
  {
    return;
  }
  if ( !m_bSelection )
  {
    return;
  }
  size_t     iSelectionStartX    = m_iSelectionAnchorX,
          iSelectionStartY    = m_iSelectionAnchorY,
          iSelectionEndX      = m_iCursorX,
          iSelectionEndY      = m_iCursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_iCursorX;
    iSelectionStartY    = m_iCursorY;
    iSelectionEndX      = m_iSelectionAnchorX;
    iSelectionEndY      = m_iSelectionAnchorY;
  }
  if ( iSelectionStartY == iSelectionEndY )
  {
    // Spezialfall, Selection innerhalb einer Zeile
    m_vectText[iSelectionStartY].erase( iSelectionStartX, iSelectionEndX - iSelectionStartX );
    m_iCursorX = iSelectionStartX;
  }
  else
  {
    // Anfang der Selection rausnehmen
    m_vectText[iSelectionStartY] = m_vectText[iSelectionStartY].substr( 0, iSelectionStartX );
    // Endteil der Selection rausnehmen
    m_vectText[iSelectionEndY] = m_vectText[iSelectionEndY].substr( iSelectionEndX );

    m_vectText[iSelectionStartY] += m_vectText[iSelectionEndY];

    std::vector<GR::String>::iterator      itSelEnd = GetStringIteratorAt( iSelectionEndY );
    m_vectText.erase( itSelEnd );

    // dazwischenliegende Zeilen entfernen
    if ( iSelectionEndY - iSelectionStartY >= 2 )
    {
      std::vector<GR::String>::iterator    itBegin = GetStringIteratorAt( iSelectionStartY + 1 ),
                                            itEnd   = GetStringIteratorAt( iSelectionEndY );
      m_vectText.erase( itBegin, itEnd );
    }
    m_iCursorX = iSelectionStartX;
    m_iCursorY = iSelectionStartY;
  }
  m_iSelectionAnchorX = m_iCursorX;
  m_iSelectionAnchorY = m_iCursorY;
  m_bModified = TRUE;
  m_bSelection = FALSE;

}



void CScriptEditCtrl::InsertChar( char cChar, BOOL bRedraw )
{

  GR::String     strDummy = "";
  
  strDummy += cChar;

  InsertString( strDummy, bRedraw );

}



void CScriptEditCtrl::InsertString( const GR::String& strMyNew, BOOL bRedraw, BOOL bSelect )
{

  if ( !m_bEnabled )
  {
    return;
  }

  if ( m_iCursorY > m_vectText.size() + 1 )
  {
    return;
  }
  if ( ( m_vectText.size() == 0 )
  &&   ( m_iCursorY == 0 ) )
  {
    // diesen String gibt es (noch) gar nicht
    m_vectText.push_back( strMyNew );
  }
  else
  {
    GR::String     strResult = "";

    GR::String     strNew = strMyNew;
    

    if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
    {
      GR::String   strDummy = "";
      for ( size_t i = 0; i < strNew.length(); i++ )
      {
        if ( strNew[i] == VK_TAB )
        {
          for ( DWORD j = 0; j < m_dwTabSpacing; j++ )
          {
            strDummy += ' ';
          }
        }
        else
        {
          strDummy += strNew[i];
        }
      }
      strNew.swap( strDummy );
    }
    for ( size_t i = 0; i < strNew.length(); i++ )
    {
      if ( strNew[i] == 10 )
      {
        // ein Zeilenumbruch ist da drin!
        if ( m_iCursorX < (int)m_vectText[m_iCursorY].length() )
        {
          // Zeilenumbruch einfügen
          std::vector<GR::String>::iterator    itVectString;
          size_t                                   iDummy = m_iCursorY;
          GR::String     strDummy = "";

      
          itVectString = m_vectText.begin();
          while ( iDummy )
          {
            itVectString++;
            iDummy--;
          }
          m_vectText.insert( itVectString, "" );
          m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1].substr( 0, m_iCursorX );
          m_iCursorY++;
          m_vectText[m_iCursorY] = m_vectText[m_iCursorY].substr( m_iCursorX );
          m_iCursorX = 0;
        }
        else
        {
          // Zeilenumbruch einfügen
          std::vector<GR::String>::iterator    itVectString;
          GR::String     strDummy = "";

      
          itVectString = m_vectText.begin() + m_iCursorY;
          m_vectText.insert( itVectString, "" );
          m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1].substr( 0, m_iCursorX );
          m_iCursorY++;
          m_vectText[m_iCursorY] = "";
          m_iCursorX = 0;
        }
      }
      else if ( ( (BYTE)strNew[i] >= 32 )
      ||        ( (BYTE)strNew[i] == VK_TAB ) )
      {
        // Zeichen einfügen
        strResult = "";
        if ( m_iCursorX > 0 )
        { 
          strResult = m_vectText[m_iCursorY].substr( 0, m_iCursorX );
        }
        strResult += (char)strNew[i];
        if ( m_iCursorX < (int)m_vectText[m_iCursorY].length() )
        {
          strResult += m_vectText[m_iCursorY].substr( m_iCursorX );
        }
        m_vectText[m_iCursorY] = strResult;
        m_iCursorX++;
      }
    }
  }
  m_bModified = TRUE;
  AdjustScrollBars();

  UpdateSelectionAnchor( bSelect );
  Invalidate();

}



void CScriptEditCtrl::UpdateCaret()
{

  if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    m_iCaretX = m_iCursorX;
  }
  else
  {
    m_iCaretX = 0;
    for ( size_t i = 0; i < m_iCursorX; i++ )
    {
      if ( m_vectText[m_iCursorY].at( i ) == VK_TAB )
      {
        m_iCaretX += m_dwTabSpacing;
      }
      else
      {
        m_iCaretX++;
      }
    }
  }

}



void CScriptEditCtrl::DrawCursor()
{

  UpdateCaret();
  if ( m_bFocus )
  {
    if ( !m_bShowCaret )
    {
      ShowCaret();
      m_bShowCaret = TRUE;
    }
    POINT   ptCaret;

    ptCaret.x = (LONG)( ( m_iCaretX - m_iFirstCharacter ) * m_iCharWidth );
    ptCaret.y = (LONG)( ( m_iCursorY - m_iFirstLine ) * m_iCharHeight );
    SetCaretPos( ptCaret );
  }

}



void CScriptEditCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{

  if ( m_bSelectingByMouse )
  {
    return;
  }
  if ( nChar == 16 )
  {
    return;
  }
  BOOL      bControlPressed = (BOOL)( GetKeyState( VK_CONTROL ) & 0xf0 ),
            bShiftPressed = (BOOL)( ( GetKeyState( VK_SHIFT ) & 0xf0 ) | ( GetKeyState( VK_CAPITAL ) & 0x01 ) );

  if ( nChar == 'V' )
  {
    if ( bControlPressed && m_bEnabled ) Paste();
    return;
  }
  else if ( nChar == 'C' )
  {
    if ( bControlPressed ) Copy();
    return;
  }
  else if ( nChar == 'X' )
  {
    if ( bControlPressed && m_bEnabled ) Cut();
    return;
  }
  else
    // {
    // if ( bControlPressed )
    // {
      // if ( !m_bEnabled )
      // {
        // return;
      // }

      // // Paste from Clipboard
      // if ( !IsClipboardFormatAvailable( CF_TEXT ) ) 
      // {
        // return;
      // }
        
// 	    if ( OpenClipboard() )
// 	    {
        // RemoveSelection();
        // char        *cBuffer,
                    // cDummy;
        // HANDLE      hData = GetClipboardData( CF_TEXT );

// 	      cBuffer = (char*)GlobalLock( hData );
        // if ( cBuffer != NULL )
        // {
          // GR::String   strDummy = "";
          // int           iPos = 0;
          // while ( TRUE )
          // {
            // cDummy = cBuffer[iPos];
            // if ( cDummy == 0 )
            // {
              // InsertString( strDummy, FALSE );
              // break;
            // }
            // if ( cDummy == 10 )
            // {
              // // String mit Zeilenumbruch!
              // std::vector<GR::String>::iterator    itVectString;
              // int                                   iDummy = m_iCursorY;

      
              // itVectString = m_vectText.begin();
              // while ( iDummy )
              // {
                // itVectString++;
                // iDummy--;
              // }
              // m_vectText.insert( itVectString );
              // if ( m_iCursorX == m_vectText[m_iCursorY + 1].length() )
              // {
                // m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1] + strDummy;
                // m_iCursorY++;
                // m_vectText[m_iCursorY] = "";
              // }
              // else
              // {
                // m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1].substr( 0, m_iCursorX ) + strDummy;
                // m_iCursorY++;
                // if ( m_vectText[m_iCursorY].length() > 0 )
                // {
                  // m_vectText[m_iCursorY] = m_vectText[m_iCursorY].substr( m_iCursorX );
                // }
                // else
                // {
                  // m_vectText[m_iCursorY] = "";
                // }
                // m_iCursorX = 0;
              // }
              // strDummy = "";
            // }
            // else if ( cDummy != 13 )
            // {
              // if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
              // {
                // if ( cDummy == VK_TAB )
                // {
                  // for ( DWORD i = 0; i < m_dwTabSpacing; i++ )
                  // {
                    // strDummy += ' ';
                  // }
                  // iPos++;
                  // continue;
                // }
              // }
              // strDummy += cDummy;
            // }
            // iPos++;
          // }
        // }
        // GlobalUnlock( hData );
        // m_bModified = TRUE;
// 	      CloseClipboard();
        // ScrollIntoView();
        // Invalidate();
        // return;
// 	    }
    // }
  // }
  // else if ( nChar == 'C' )
  // {
    // if ( bControlPressed && m_bSelection )
    // {
      // // Copy to Clipboard
      // if ( m_vectText.size() > 0 )
      // {
        // int             iX1,
                        // iY1,
                        // iX2,
                        // iY2;
        // GR::String     strDummy;

        // if ( m_bSelection )
        // {
          // iX1 = m_iSelectionAnchorX;
          // iY1 = m_iSelectionAnchorY;
          // iX2 = m_iCursorX;
          // iY2 = m_iCursorY;
          // if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
          // {
            // // Koordinaten tauschen
            // iX1 = m_iCursorX;
            // iY1 = m_iCursorY;
            // iX2 = m_iSelectionAnchorX;
            // iY2 = m_iSelectionAnchorY;
          // }
        // }
        // else
        // {
          // iX1 = 0;
          // iY1 = 0;
          // iX2 = m_vectText[m_vectText.size() - 1].length();
          // iY2 = m_vectText.size() - 1;
        // }
        // if ( ( iX1 == iX2 )
        // &&   ( iY1 == iY2 ) )
        // {
          // // nix da!
          // if ( OpenClipboard() )
          // {
            // EmptyClipboard();
            // CloseClipboard();
          // }
          // return;
        // }
// 	      if ( OpenClipboard() )
// 	      {
          // int   iChar = iX1,
                // iZeile = iY1;

          // strDummy = "";
          // do
          // {
            // if ( iZeile < m_vectText.size() )
            // {
              // if ( iChar < m_vectText[iZeile].length() )
              // {
                // strDummy += m_vectText[iZeile][iChar];
              // }
              // iChar++;
              // if ( ( iZeile < iY2 )
              // ||   ( ( iZeile == iY2 )
              // &&     ( iChar < iX2 ) ) )
              // {
                // if ( iChar >= m_vectText[iZeile].length() )
                // {
                  // // Zeilenumbruch!
                  // strDummy += "\r\n";
                  // iChar = 0;
                  // iZeile++;
                // }
              // }
            // }
            // if ( ( iZeile == iY2 )
            // &&   ( iChar >= iX2 ) )
            // {
              // break;
            // }
          // }
          // while ( TRUE );

          // strDummy += '\0';

// 		      HGLOBAL clipbuffer;
// 		      char * buffer;
// 		      EmptyClipboard();
// 		      clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strDummy.length() + 1 );
// 		      buffer = (char*)GlobalLock( clipbuffer );
// 		      strcpy( buffer, strDummy.c_str() );
// 		      GlobalUnlock( clipbuffer );
// 		      SetClipboardData( CF_TEXT, clipbuffer );
// 		      CloseClipboard();
        // }
      // }
    // }

  // }
  // else if ( nChar == 'X' )
  // {
    // if ( bControlPressed && m_bSelection )
    // {
      // // Cut to Clipboard
      // if ( m_vectText.size() > 0 )
      // {
        // int             iX1,
                        // iY1,
                        // iX2,
                        // iY2;
        // GR::String     strDummy;

        // if ( m_bSelection )
        // {
          // iX1 = m_iSelectionAnchorX;
          // iY1 = m_iSelectionAnchorY;
          // iX2 = m_iCursorX;
          // iY2 = m_iCursorY;
          // if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
          // {
            // // Koordinaten tauschen
            // iX1 = m_iCursorX;
            // iY1 = m_iCursorY;
            // iX2 = m_iSelectionAnchorX;
            // iY2 = m_iSelectionAnchorY;
          // }
        // }
        // else
        // {
          // iX1 = 0;
          // iY1 = 0;
          // iX2 = m_vectText[m_vectText.size() - 1].length();
          // iY2 = m_vectText.size() - 1;
        // }
        // if ( ( iX1 == iX2 )
        // &&   ( iY1 == iY2 ) )
        // {
          // // nix da!
          // if ( OpenClipboard() )
          // {
            // EmptyClipboard();
            // CloseClipboard();
          // }
          // return;
        // }
// 	      if ( OpenClipboard() )
// 	      {
          // int   iChar = iX1,
                // iZeile = iY1;

          // strDummy = "";
          // do
          // {
            // if ( iZeile < m_vectText.size() )
            // {
              // if ( iChar < m_vectText[iZeile].length() )
              // {
                // strDummy += m_vectText[iZeile][iChar];
              // }
              // iChar++;
              // if ( ( iZeile < iY2 )
              // ||   ( ( iZeile == iY2 )
              // &&     ( iChar < iX2 ) ) )
              // {
                // if ( iChar >= m_vectText[iZeile].length() )
                // {
                  // // Zeilenumbruch!
                  // strDummy += "\r\n";
                  // iChar = 0;
                  // iZeile++;
                // }
              // }
            // }
            // if ( ( iZeile == iY2 )
            // &&   ( iChar >= iX2 ) )
            // {
              // break;
            // }
          // }
          // while ( TRUE );

          // strDummy += '\0';

// 		      HGLOBAL clipbuffer;
// 		      char * buffer;
// 		      EmptyClipboard();
// 		      clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strDummy.length() + 1 );
// 		      buffer = (char*)GlobalLock( clipbuffer );
// 		      strcpy( buffer, strDummy.c_str() );
// 		      GlobalUnlock( clipbuffer );
// 		      SetClipboardData( CF_TEXT, clipbuffer );
// 		      CloseClipboard();

          // if ( m_bSelection )
          // {
            // RemoveSelection();
          // }
          // else
          // {
            // Reset();
          // }
          // m_bModified = TRUE;
          // AdjustScrollBars();
          // Invalidate();
        // }
      // }
    // }

  // }
  if ( nChar == 8 )
  {
    // BackSpace!
    if ( !m_bEnabled )
    {
      return;
    }
    if ( m_bSelection )
    {
      RemoveSelection();
      AdjustScrollBars();
    }
    else
    {
      if ( m_iCursorX > 0 )
      {
        m_vectText[m_iCursorY].erase( m_iCursorX - 1, 1 );
        m_iCursorX--;

        AdjustScrollBars();
        ScrollIntoView();
        Invalidate();
      }
      else
      {
        // Zeilen zusammenlegen?
        if ( m_iCursorY > 0 )
        {
          m_iCursorY--;
          m_iCursorX = m_vectText[m_iCursorY].length();
          m_vectText[m_iCursorY] += m_vectText[m_iCursorY + 1];

          std::vector<GR::String>::iterator      itSelDummy = GetStringIteratorAt( m_iCursorY + 1 );
          m_vectText.erase( itSelDummy );

          AdjustScrollBars();
          ScrollIntoView();
          Invalidate();
        }
      }
    }
    m_bModified = TRUE;
  }
  if ( nChar == VK_LEFT )
  {
    if ( bControlPressed )
    {
      // Word-Hüpf
      if ( m_iCursorX == 0 )
      {
        if ( m_iCursorY > 0 )
        {
          // zum Ende der vorherigen Zeile
          m_iCursorWantedX = -1;
          m_iCursorY--;
          m_iCursorX = m_vectText[m_iCursorY].length();

          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_bSelection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
      else
      {
        size_t     iPos = m_iCursorX;
        BOOL    bAlphaCharFound = FALSE;
        if ( iPos >= m_vectText[m_iCursorY].length() )
        {
          iPos--;
        }
        do
        {
          iPos--;
          if ( iPos < 0 )
          {
            break;
          }
          if ( isalnum( m_vectText[m_iCursorY][iPos] ) )
          {
            bAlphaCharFound = TRUE;
          }
          else
          {
            if ( bAlphaCharFound )
            {
              break;
            }
          }
        }
        while ( TRUE );
        m_iCursorX = iPos + 1;
        m_iCursorWantedX = -1;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_bSelection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
    }
    else
    {
      if ( m_iCursorX > 0 )
      {
        m_iCursorWantedX = -1;
        m_iCursorX--;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_bSelection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
      else
      {
        if ( m_iCursorY > 0 )
        {
          m_iCursorWantedX = -1;
          m_iCursorY--;
          m_iCursorX = m_vectText[m_iCursorY].length();
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_bSelection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
    }
  }
  else if ( nChar == VK_RIGHT )
  {
    if ( bControlPressed )
    {
      // Word-Hüpf
      if ( m_iCursorX >= (int)m_vectText[m_iCursorY].length() )
      {
        if ( m_iCursorY + 1 < (int)m_vectText.size() )
        {
          // zum Anfang der nächsten Zeile
          m_iCursorWantedX = -1;
          m_iCursorY++;
          m_iCursorX = 0;
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_bSelection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
      else
      {
        size_t     iPos = m_iCursorX;
        BOOL    bOutofAlphaChar = FALSE;
        do
        {
          iPos++;
          if ( iPos >= m_vectText[m_iCursorY].length() )
          {
            m_iCursorX = m_vectText[m_iCursorY].length();
            break;
          }
          if ( isalnum( m_vectText[m_iCursorY][iPos] ) )
          {
            if ( bOutofAlphaChar )
            {
              m_iCursorX = iPos;
              break;
            }
          }
          else
          {
            bOutofAlphaChar = TRUE;
          }
        }
        while ( TRUE );
        m_iCursorWantedX = -1;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_bSelection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
    }
    else
    {
      if ( m_iCursorX < m_vectText[m_iCursorY].length() )
      {
        m_iCursorWantedX = -1;
        m_iCursorX++;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_bSelection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
      else
      {
        if ( m_iCursorY + 1 < m_vectText.size() )
        {
          m_iCursorWantedX = -1;
          m_iCursorY++;
          m_iCursorX = 0;
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_bSelection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
    }
  }
  else if ( nChar == VK_UP )
  {
    if ( m_iCursorY > 0 )
    {
      m_iCursorY--;
      if ( m_iCursorWantedX == -1 )
      {
        m_iCursorWantedX = m_iCursorX;
      }
      else
      {
        m_iCursorX = m_iCursorWantedX;
      }
      if ( m_iCursorX > m_vectText[m_iCursorY].length() )
      {
        m_iCursorX = m_vectText[m_iCursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_bSelection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == VK_DOWN )
  {
    if ( m_iCursorY + 1 < m_vectText.size() )
    {
      m_iCursorY++;
      if ( m_iCursorWantedX == -1 )
      {
        m_iCursorWantedX = m_iCursorX;
      }
      else
      {
        m_iCursorX = m_iCursorWantedX;
      }
      if ( m_iCursorX > m_vectText[m_iCursorY].length() )
      {
        m_iCursorX = m_vectText[m_iCursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_bSelection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == 33 )
  {
    // Page-Up
    if ( !bControlPressed )
    {
      if ( m_iCursorY >= m_iVisibleLines * 3 / 4 )
      {
        m_iCursorY -= m_iVisibleLines * 3 / 4;
      }
      else
      {
        m_iCursorY = 0;
      }
      if ( m_iCursorWantedX == -1 )
      {
        m_iCursorWantedX = m_iCursorX;
      }
      else
      {
        m_iCursorX = m_iCursorWantedX;
      }
      if ( m_iCursorX > m_vectText[m_iCursorY].length() )
      {
        m_iCursorX = m_vectText[m_iCursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_bSelection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == 34 )
  {
    // Page-Down
    if ( !bControlPressed )
    {
      m_iCursorY += m_iVisibleLines * 3 / 4;
      if ( m_iCursorY >= m_vectText.size() )
      {
        m_iCursorY = m_vectText.size() - 1;
      }
      if ( m_iCursorWantedX == -1 )
      {
        m_iCursorWantedX = m_iCursorX;
      }
      else
      {
        m_iCursorX = m_iCursorWantedX;
      }
      if ( m_iCursorX > m_vectText[m_iCursorY].length() )
      {
        m_iCursorX = m_vectText[m_iCursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_bSelection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == VK_HOME )
  {
    if ( bControlPressed )
    {
      m_iCursorX = 0;
      m_iCursorY = 0;
      ScrollIntoView();
      UpdateSelectionAnchor( TRUE );
      Invalidate();
    }
    else
    {
      if ( m_iCursorX > 0 )
      {
        m_iCursorWantedX = -1;
        m_iCursorX = 0;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_bSelection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
    }
  }
  else if ( nChar == VK_END )
  {
    if ( bControlPressed )
    {
      if ( m_vectText.size() > 0 )
      {
        m_iCursorX = m_vectText[m_vectText.size() - 1].length();
        m_iCursorY = m_vectText.size() - 1;
        ScrollIntoView();
        UpdateSelectionAnchor( TRUE );
        Invalidate();
      }
    }
    else
    {
      m_iCursorWantedX = -1;
      m_iCursorX = m_vectText[m_iCursorY].length();
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_bSelection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == VK_RETURN )
  {
    if ( !m_bEnabled )
    {
      if ( m_iCursorY + 1 < (int)m_vectText.size() )
      {
        m_iCursorY++;
        m_iCursorX = 0;
      }
      m_bModified = TRUE;
      AdjustScrollBars();
      ScrollIntoView();
      UpdateSelectionAnchor( TRUE );
      Invalidate();
      return;
    }
    m_bModified = TRUE;
    RemoveSelection();
    if ( m_iCursorX < (int)m_vectText[m_iCursorY].length() )
    {
      // Zeilenumbruch einfügen
      std::vector<GR::String>::iterator    itVectString;
      size_t                                   iDummy = m_iCursorY;
      GR::String     strDummy = "";

      
      itVectString = m_vectText.begin();
      while ( iDummy )
      {
        itVectString++;
        iDummy--;
      }
      m_vectText.insert( itVectString, "" );
      m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1].substr( 0, m_iCursorX );
      m_iCursorY++;
      m_vectText[m_iCursorY] = m_vectText[m_iCursorY].substr( m_iCursorX );

      int     iIndent = 0;
      if ( m_vectText[m_iCursorY - 1].length() > 0 )
      {
        while ( m_vectText[m_iCursorY - 1].at( iIndent ) == ' ' )
        {
          iIndent++;
          if ( iIndent >= (int)m_vectText[m_iCursorY - 1].length() )
          {
            break;
          }
        }
        for ( int i = 0; i < iIndent; i++ )
        {
          m_vectText[m_iCursorY] = ' ' + m_vectText[m_iCursorY];
        }
      }

      m_iCursorX = iIndent;
    }
    else
    {
      // Leerzeile einfügen
      std::vector<GR::String>::iterator    itVectString;
      size_t                                   iDummy = m_iCursorY;
      GR::String     strDummy = "";

      itVectString = m_vectText.begin();
      while ( iDummy )
      {
        itVectString++;
        iDummy--;
      }
      m_vectText.insert( itVectString, "" );
      m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1];
      m_iCursorY++;
      m_vectText[m_iCursorY] = "";

      
      int     iIndent = 0;
      if ( m_vectText[m_iCursorY - 1].length() > 0 )
      {
        while ( m_vectText[m_iCursorY - 1].at( iIndent ) == ' ' )
        {
          iIndent++;
          if ( iIndent >= (int)m_vectText[m_iCursorY - 1].length() )
          {
            break;
          }
        }
        for ( int i = 0; i < iIndent; i++ )
        {
          m_vectText[m_iCursorY] = ' ' + m_vectText[m_iCursorY];
        }
      }

      m_iCursorX = iIndent;

    }
    AdjustScrollBars();
    ScrollIntoView();
    UpdateSelectionAnchor( TRUE );
    Invalidate();
  }
  else if ( nChar == VK_DELETE )
  {
    // Delete
    if ( !m_bEnabled )
    {
      return;
    }
    m_bModified = TRUE;
    if ( m_bSelection )
    {
      RemoveSelection();
    }
    else
    {
      if ( m_iCursorX < (int)m_vectText[m_iCursorY].length() )
      {
        // ein Zeichen löschen
        m_vectText[m_iCursorY].erase( m_iCursorX, 1 );
      }
      else
      {
        // Zeilen zusammenlegen?
        if ( m_iCursorY + 1 < (int)m_vectText.size() )
        {
          m_vectText[m_iCursorY] += m_vectText[m_iCursorY + 1];

          std::vector<GR::String>::iterator      itSelDummy = GetStringIteratorAt( m_iCursorY + 1 );
          m_vectText.erase( itSelDummy );
        }
      }
    }
    AdjustScrollBars();
    ScrollIntoView();
    Invalidate();
  }

}



BOOL CScriptEditCtrl::ScrollIntoView()
{

  if ( !IsWindowVisible() )
  {
    return FALSE;
  }

  BOOL        bHorzChanged = FALSE,
              bVertChanged = FALSE,
              bScrollHorz = TRUE,
              bScrollVert = TRUE;


  ClipCoordinatesToText( &m_iCursorX, &m_iCursorY );
  if ( (int)m_vectText.size() < m_iVisibleLines )
  {
    bScrollVert = FALSE;
  }

  UpdateCaret();

  while ( m_iCaretX < m_iFirstCharacter )
  {
    m_iFirstCharacter--;
    if ( m_iFirstCharacter < 0 )
    {
      m_iFirstCharacter = 0;
    }
    bHorzChanged = TRUE;
  }
  while ( m_iCaretX > m_iFirstCharacter + m_iVisibleCharsPerLine )
  {
    m_iFirstCharacter++;
    bHorzChanged = TRUE;
  }
  /*
  while ( m_iCursorX < m_iFirstCharacter )
  {
    m_iFirstCharacter--;
    if ( m_iFirstCharacter < 0 )
    {
      m_iFirstCharacter = 0;
    }
    bHorzChanged = TRUE;
  }
  while ( m_iCursorX > m_iFirstCharacter + m_iVisibleCharsPerLine )
  {
    m_iFirstCharacter++;
    bHorzChanged = TRUE;
  }
  */
  if ( bHorzChanged )
  {
    SetScrollPos( SB_HORZ, (int)m_iFirstCharacter );
  }
  if ( bScrollVert )
  {
    while ( m_iCursorY < m_iFirstLine )
    {
      m_iFirstLine--;
      bVertChanged = TRUE;
    }
    while ( m_iCursorY >= m_iFirstLine + m_iVisibleLines )
    {
      m_iFirstLine++;
      bVertChanged = TRUE;
    }

    if ( bVertChanged )
    {
      SetScrollPos( SB_VERT, (int)m_iFirstLine );
    }
  }
  if ( ( bHorzChanged )
  ||   ( bVertChanged ) )
  {
    return TRUE;
  }
  return FALSE;

}



void CScriptEditCtrl::AdjustScrollBars()
{

  SetScrollRange( SB_HORZ, 0, SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH );
  if ( (int)m_vectText.size() > m_iVisibleLines )
  {
    SetScrollRange( SB_VERT, 0, (int)( m_vectText.size() - m_iVisibleLines ) );
  }
  else
  {
    SetScrollRange( SB_VERT, 0, 0 );
    m_iFirstLine = 0;
  }

  // Werte anpassen
  GetClientRect( &m_rectRC );
  m_iVisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_iCharWidth;
  m_iVisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_iCharHeight;

  // nochmal, falls sich die Randwerte geändert haben
  if ( (int)m_vectText.size() > m_iVisibleLines )
  {
    SetScrollRange( SB_VERT, 0, (int)( m_vectText.size() - m_iVisibleLines ) );
  }
  else
  {
    SetScrollRange( SB_VERT, 0, 0 );
    m_iFirstLine = 0;
  }

}



void CScriptEditCtrl::UpdateSelectionAnchor( BOOL bTestShift )
{

	if ( bTestShift )
	{
    //BOOL bShiftPressed = (BOOL)( ( GetKeyState( VK_SHIFT ) & 0xf0 ) | ( GetKeyState( VK_CAPITAL ) & 0x01 ) );
    BOOL bShiftPressed = (BOOL)( GetKeyState( VK_SHIFT ) & 0xf0 );
		if ( !bShiftPressed )
		{
      m_iSelectionAnchorX = m_iCursorX;
      m_iSelectionAnchorY = m_iCursorY;
    }
    else
    {
      if ( !m_bSelection )
      {
        m_bSelection = TRUE;
        Invalidate();
      }
    }
	}
	else
	{
    m_iSelectionAnchorX = m_iCursorX;
    m_iSelectionAnchorY = m_iCursorY;
    if ( m_bSelection )
    {
      m_bSelection = FALSE;
      Invalidate();
    }
	}
  if ( m_bSelection )
  {
    if ( ( m_iCursorX == m_iSelectionAnchorX )
    &&   ( m_iCursorY == m_iSelectionAnchorY ) )
    {
      m_bSelection = FALSE;
      Invalidate();
    }
  }

}



void CScriptEditCtrl::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{

  if ( m_bSelectingByMouse )
  {
    return;
  }
  BOOL      bControlPressed = (BOOL)( GetKeyState( VK_CONTROL ) & 0xf0 ),
            bShiftPressed = (BOOL)( ( GetKeyState( VK_SHIFT ) & 0xf0 ) | ( GetKeyState( VK_CAPITAL ) & 0x01 ) );


  if ( ( bControlPressed )
  &&   ( nChar != '{' )
  &&   ( nChar != '[' )
  &&   ( nChar != '@' )
  &&   ( nChar != '\\' )
  &&   ( nChar != '|' )
  &&   ( nChar != '~' )
  &&   ( nChar != '²' )
  &&   ( nChar != '³' )
  &&   ( nChar != ']' )
  &&   ( nChar != '}' ) )
  {
    return;
  }
  if ( ( (BYTE)nChar < 32 )
  &&   ( nChar != VK_TAB ) )
  {
    return;
  }
  RemoveSelection();
  InsertChar( nChar, TRUE );
  ScrollIntoView();

}



void CScriptEditCtrl::OnMouseMove( UINT nFlags, POINT point ) 
{

  if ( m_bSelectingByMouse )
  {
    if ( !PtInRect( &m_rectRC, point ) )
    {
      return;
    }
    size_t     iX,
                iY;

    GetMouseCursorPos( point, &iX, &iY );

    m_iCursorX = iX;
    m_iCursorY = iY;
    if ( ( m_iCursorX != m_iSelectionAnchorX )
    ||   ( m_iCursorY != m_iSelectionAnchorY ) )
    {
      m_bSelection = TRUE;
    }
    else
    {
      m_bSelection = FALSE;
    }
    Invalidate();
  }

	CWnd::OnMouseMove( nFlags, point );

}



void CScriptEditCtrl::OnLButtonUp( UINT nFlags, POINT point ) 
{

  if ( m_bSelectingByMouse )
  {
  	m_bSelectingByMouse = FALSE;
    ReleaseCapture();
  }
	
	CWnd::OnLButtonUp( nFlags, point );

}


//- Peter war's (isalnum erkennt keine Ümläütö)
inline bool IsAlphanumeric( const char c ) 
{
  static GR::String CharSet( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZäöüÄÖÜß" );
  if ( CharSet.find( c ) != GR::String::npos )  return true;
  else                                           return false;
}



void CScriptEditCtrl::OnLButtonDblClk( UINT nFlags, POINT point ) 
{

	SetFocus();
  if ( !m_bEnabled )
  {
    return;
  }


  size_t     iX,
          iY;

  GetMouseCursorPos( point, &iX, &iY );

  m_iCursorX = iX;
  m_iCursorY = iY;


  size_t iStartX = iX,
      iEndX = iX;

  do
  {
    if ( iStartX >= 0 )
    {
      // if ( isalnum( m_vectText[iY][iStartX] ) )
      if ( IsAlphanumeric( m_vectText[iY][iStartX] ) )
      {
        iStartX--;
      }
      else
      {
        break;
      }
    }
    else
    {
      break;
    }
  }
  while ( TRUE );
  do
  {
    if ( iEndX < (int)m_vectText[iY].length() )
    {
      // if ( isalnum( m_vectText[iY][iEndX] ) )
      if ( IsAlphanumeric( m_vectText[iY][iEndX] ) )
      {
        iEndX++;
      }
      else
      {
        break;
      }
    }
    else
    {
      break;
    }
  }
  while ( TRUE );
  if ( iStartX < iEndX )
  {
    m_bSelection = TRUE;
    m_iCursorX = iEndX;
    m_iCursorY = iY;
    m_iSelectionAnchorX = iStartX + 1;
    m_iSelectionAnchorY = iY;
    m_iCursorWantedX = -1;
    Invalidate();
  }
	
	//CWnd::OnLButtonDblClk( nFlags, point );

}



void CScriptEditCtrl::GetMouseCursorPos( POINT point, size_t *iX, size_t *iY )
{

  *iY = m_iFirstLine + point.y / m_iCharHeight;
  if ( *iY >= (int)m_vectText.size() )
  {
    *iY = m_vectText.size() - 1;
  }
  if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    *iX = m_iFirstCharacter + ( ( point.x + m_iCharWidth / 2 ) ) / m_iCharWidth;
    // Cursor im Text?
    if ( *iX > (int)m_vectText[*iY].length() )
    {
      *iX = m_vectText[*iY].length();
    }
  }
  else
  {
    if ( m_vectText[*iY].length() == 0 )
    {
      *iX = 0;
      return;
    }

    size_t   iDummyX,
          iRealX,
          iWidth;

    iDummyX = 0;
    iRealX = 0;

    do
    {
      if ( m_vectText[*iY][iDummyX] == VK_TAB )
      {
        iWidth = m_iCharWidth * m_dwTabSpacing;
      }
      else
      {
        iWidth = m_iCharWidth;
      }
      if ( ( m_iFirstCharacter * m_iCharWidth + point.x >= iRealX )
      &&   ( m_iFirstCharacter * m_iCharWidth + point.x < iRealX + iWidth ) )
      {
        // hierhin haben wir geklickt!
        if ( m_iFirstCharacter * m_iCharWidth + point.x - iRealX > iWidth / 2 )
        {
          // ans Ende des Buchstabens
          *iX = iDummyX + 1;
          if ( *iX > (int)m_vectText[*iY].length() )
          {
            *iX = m_vectText[*iY].length();
          }
        }
        else
        {
          *iX = iDummyX;
        }
        return;
      }

      iDummyX++;
      if ( iDummyX >= (int)m_vectText[*iY].length() )
      {
        // über den Text raus!
        *iX = m_vectText[*iY].length();
        return;
      }
      iRealX += iWidth;
    }
    while ( TRUE );
  }

}



void CScriptEditCtrl::OnLButtonDown( UINT nFlags, POINT point )
{

	SetFocus();

  size_t     iX,
          iY;


  GetMouseCursorPos( point, &iX, &iY );

  m_iCursorX = iX;
  m_iCursorY = iY;
  if ( !m_bSelectingByMouse )
  {
    m_bSelectingByMouse = TRUE;
    m_iSelectionAnchorX = iX;
    m_iSelectionAnchorY = iY;
    SetCapture();
  }

  ScrollIntoView();
  Invalidate();
	
	//CWnd::OnLButtonDown( nFlags, point );

}



void CScriptEditCtrl::OnSetFocus( HWND hwndOld ) 
{

	//CWnd::OnSetFocus( pOldWnd );
	
  if ( !m_bFocus )
  {
	  m_bFocus = TRUE;
  }
  if ( !m_bShowCaret )
  {
    CreateSolidCaret( 1, (int)m_iCharHeight );
    ShowCaret();
    m_bShowCaret = TRUE;
  }
  DrawCursor();
  Invalidate();
	
}



void CScriptEditCtrl::OnKillFocus( HWND hwndNew ) 
{

	//CWnd::OnKillFocus( pNewWnd );
	
  if ( m_bSelectingByMouse )
  {
    m_bSelectingByMouse = FALSE;
    ReleaseCapture();
  }

  if ( m_bFocus )
  {
	  m_bFocus = FALSE;
    Invalidate();
  }
  if ( m_bShowCaret )
  {
    m_bShowCaret = FALSE;
    HideCaret();
  }
	
}



UINT CScriptEditCtrl::OnGetDlgCode() 
{

	return DLGC_WANTALLKEYS;
	
}




void CScriptEditCtrl::OnSetCursor( HWND hWnd, UINT nHitTest, UINT message )
{

  if ( nHitTest == HTCLIENT )
  {
    SetCursor( LoadCursor( NULL, IDC_IBEAM ) );
  }
  else
  {
    //CWnd::OnSetCursor( pWnd, nHitTest, message );
  }

}


	
void CScriptEditCtrl::OnVScroll( UINT nSBCode, UINT nPos ) 
{
	
  int     iMin,
          iMax;


  GetScrollRange( SB_VERT, &iMin, &iMax );
  if ( iMax == 0 )
  {
    return;
  }
	switch ( nSBCode )
	{
	  case SB_LEFT:
      m_iFirstLine = 0;
		  Invalidate();
		  break;
	  case SB_LINELEFT:
      if ( m_iFirstLine > 0 )
      {
        m_iFirstLine--;
  		  Invalidate();
      }
		  break;
	  case SB_PAGELEFT:
      m_iFirstLine -= m_iVisibleLines * 3 / 4;
      if ( m_iFirstLine < 0 )
      {
        m_iFirstLine = 0;
      }
		  Invalidate();
		  break;
	  case SB_LINERIGHT:
      if ( m_iFirstLine < (int)m_vectText.size() - m_iVisibleLines )
      {
        m_iFirstLine++;
  		  Invalidate();
      }
		  break;
	  case SB_PAGERIGHT:
      m_iFirstLine += m_iVisibleLines * 3 / 4;
      if ( m_iFirstLine > (int)m_vectText.size() - m_iVisibleLines )
      {
        m_iFirstLine = m_vectText.size() - m_iVisibleLines;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_iFirstLine = m_vectText.size() - m_iVisibleLines;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      m_iFirstLine = nPos;
		  Invalidate();
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}
	SetScrollPos( SB_VERT, (int)m_iFirstLine, TRUE );

	//CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

}



void CScriptEditCtrl::OnHScroll(UINT nSBCode, UINT nPos )
{
	
	switch( nSBCode )
	{
	  case SB_LEFT:
      m_iFirstCharacter = 0;
		  Invalidate();
		  break;
	  case SB_LINELEFT:
      if ( m_iFirstCharacter > 0 )
      {
        m_iFirstCharacter--;
  		  Invalidate();
      }
		  break;
	  case SB_PAGELEFT:
      m_iFirstCharacter -= m_iVisibleCharsPerLine * 3 / 4;
      if ( m_iFirstCharacter < 0 )
      {
        m_iFirstCharacter = 0;
      }
		  Invalidate();
		  break;
	  case SB_LINERIGHT:
      if ( m_iFirstCharacter < (int)SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_iVisibleCharsPerLine )
      {
        m_iFirstCharacter++;
  		  Invalidate();
      }
		  break;
	  case SB_PAGERIGHT:
      m_iFirstCharacter += m_iVisibleCharsPerLine * 3 / 4;
      if ( m_iFirstCharacter > (int)SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_iVisibleCharsPerLine )
      {
        m_iFirstCharacter = SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_iVisibleCharsPerLine;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_iFirstCharacter = SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_iVisibleCharsPerLine;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      m_iFirstCharacter = nPos;
		  Invalidate();
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}
	SetScrollPos( SB_HORZ, (int)m_iFirstCharacter, TRUE );

  Default();

}



BOOL CScriptEditCtrl::OnEraseBkgnd( HDC hdc ) 
{
	
	return TRUE;

}



void CScriptEditCtrl::AddKeyWord( const char *szKeyWord, COLORREF colorDummy )
{

  CSyntaxColorEntry   sceDummy;


  sceDummy.strKeyWord = szKeyWord;
  sceDummy.colorKey = colorDummy;

  m_vectSyntaxEntry.push_back( sceDummy );

}



void CScriptEditCtrl::FillColorVector( GR::String strLine, size_t iCurrentLine, std::vector<COLORREF> *vectColors )
{

  // erst einmal alles schwarz
  if ( strLine.length() == 0 )
  {
    return;
  }
  for ( size_t i = 0; i < strLine.length(); i++ )
  {
    (*vectColors)[i] = 0;
  }
  if ( m_vectSyntaxEntry.size() == 0 )
  {
    return;
  }
  int     iPos,
          iWordStart,
          iWordLength;

  char    cDummy,
          cLastChar;

  DWORD   dwWordType,
          dwNewWordType;

  BOOL    bWordComplete,
          bInsideApostrophe;


  strLine += ' ';
  iPos = 0;
  iWordLength = 0;
  dwWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::DEFAULT;
  dwNewWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::DEFAULT;
  bInsideApostrophe = FALSE;
  cDummy = -1;
  do
  {
    bWordComplete = FALSE;
    cLastChar = cDummy;
    cDummy = strLine[iPos];
    iPos++;
    if ( iPos >= (int)strLine.length() )
    {
      if ( !bInsideApostrophe )
      {
        if ( dwWordType == SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD )
        {
          bWordComplete = TRUE;
        }
        dwNewWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD;
      }
    }
    if ( cDummy == '"' )
    {
      bInsideApostrophe = !bInsideApostrophe;
      if ( bInsideApostrophe )
      {
        if ( dwWordType == SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD )
        {
          bWordComplete = TRUE;
        }
        dwNewWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::DEFAULT;
      }
      else
      {
        // das abschließende Anführungszeichen auch noch einfärben
        (*vectColors)[iPos - 1] = m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::STRING_TEXT];
      }
    }
    if ( !bInsideApostrophe )
    {
      if ( ( cDummy == '/' )
      &&   ( cLastChar == '/' ) )
      {
        // ein Kommentar -> Rest der Zeile einfärben
        for ( int i = iPos - 2; i < (int)strLine.length(); i++ )
        {
          (*vectColors)[i] = m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::COMMENT];
        }
        return;
      }

      if ( ( ( cDummy >= 'a' )
      &&     ( cDummy <= 'z' ) )
      ||   ( ( cDummy >= '0' )
      &&     ( cDummy <= '9' ) )
      ||   ( cDummy == '_' )
      ||   ( ( cDummy >= 'A' )
      &&     ( cDummy <= 'Z' ) ) )
      {
        if ( dwWordType != SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD )
        {
          iWordStart = iPos - 1;
        }
        iWordLength++;
        dwNewWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD;
      }
      else
      {
        if ( dwWordType == SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD )
        {
          bWordComplete = TRUE;
        }
        dwNewWordType = SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::DEFAULT;
      }
    }
    else
    {
      (*vectColors)[iPos - 1] = m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::STRING_TEXT];
    }

    if ( bWordComplete )
    {
      if ( dwWordType == SCRIPT_EDIT_CONTROL::SYNTAX_TYPE::KEYWORD )
      {
        GR::String   strKeyWord = strLine.substr( iWordStart, iWordLength );
        for ( size_t i = 0; i < m_vectSyntaxEntry.size(); i++ )
        {
          if ( strcmp( strKeyWord.c_str(), m_vectSyntaxEntry[i].strKeyWord.c_str() ) == 0 )
          {
            // Keyword erkannt!
            for ( int j = iWordStart; j < iWordStart + iWordLength; j++ )
            {
              (*vectColors)[j] = m_vectSyntaxEntry[i].colorKey;
            }
            break;
          }
        }
        iWordLength = 0;
      }
    }
    dwWordType = dwNewWordType;
  }
  while ( iPos < (int)strLine.length() );

}



void CScriptEditCtrl::OnTimer( UINT nIDEvent ) 
{

  if ( m_bSelectingByMouse )
  {
    POINT   point;
    GetCursorPos( &point );
    ScreenToClient( &point );
    if ( point.x < m_rectRC.left )
    {
      point.x = m_rectRC.left;
    }
    if ( point.y < m_rectRC.top )
    {
      point.y = m_rectRC.top;
    }
    if ( point.x > m_rectRC.right )
    {
      point.x = m_rectRC.right;
    }
    if ( point.y > m_rectRC.bottom )
    {
      point.y = m_rectRC.bottom;
    }
    BOOL      bMoved = FALSE;
    if ( point.y - m_rectRC.top < (int)m_iCharHeight / 2 )
    {
      // Scroll nach oben
      SendMessage( WM_VSCROLL, SB_LINELEFT, 0 );
      bMoved = TRUE;
    }
    if ( m_rectRC.bottom - point.y < (int)m_iCharHeight / 2 )
    {
      // Scroll nach unten
      SendMessage( WM_VSCROLL, SB_LINERIGHT, 0 );
      bMoved = TRUE;
    }
    if ( point.x - m_rectRC.left < (int)m_iCharWidth / 2 )
    {
      // Scroll nach links
      SendMessage( WM_HSCROLL, SB_LINELEFT, 0 );
      bMoved = TRUE;
    }
    if ( m_rectRC.right - point.x < (int)m_iCharWidth / 2 )
    {
      // Scroll nach rechts
      SendMessage( WM_HSCROLL, SB_LINERIGHT, 0 );
      bMoved = TRUE;
    }
    if ( bMoved )
    {
      size_t     iX,
              iY;


      iX = m_iFirstCharacter + ( ( point.x + m_iCharWidth / 2 ) ) / m_iCharWidth;
      iY = m_iFirstLine + point.y / m_iCharHeight;

      if ( iY >= m_vectText.size() )
      {
        // unten raus, Cursor setzen
        iY = m_vectText.size() - 1;
        iX = m_vectText[iY].length();
      }
      else
      {
        // Cursor im Text?
        if ( iX > m_vectText[iY].length() )
        {
          iX = m_vectText[iY].length();
        }
      }
      m_iCursorX = iX;
      m_iCursorY = iY;
      Invalidate();
    }
  }
	
}



void CScriptEditCtrl::SetTabSpacing( DWORD dwSpaceCount )
{

  m_dwTabSpacing = dwSpaceCount;

}



void CScriptEditCtrl::Enable( BOOL bEnable )
{

  if ( bEnable )
  {
    if ( !m_bEnabled )
    {
      m_bEnabled = TRUE;
      Invalidate();
    }
  }
  else
  {
    if ( m_bEnabled )
    {
      m_bEnabled = FALSE;
      Invalidate();
    }
  }

}



BOOL CScriptEditCtrl::IsEnabled()
{

  return m_bEnabled;

}



void CScriptEditCtrl::SetColor( DWORD dwColor, COLORREF color )
{

  if ( dwColor >= m_vectColor.size() )
  {
    return;
  }
  m_vectColor[dwColor] = color;

}



void CScriptEditCtrl::ClipCoordinatesToText( size_t *iX, size_t *iY )
{

  if ( *iY >= (int)m_vectText.size() )
  {
    *iY = m_vectText.size() - 1;
  }
  if ( *iY < 0 )
  {
    *iY = 0;
  }
  if ( *iX > (int)m_vectText[*iY].length() )
  {
    *iX = m_vectText[*iY].length();
  }
  if ( *iX < 0 )
  {
    *iX = 0;
  }

}


size_t CScriptEditCtrl::GetCursorX()           const { return m_iCursorX;          }
size_t CScriptEditCtrl::GetCursorY()           const { return m_iCursorY;          }
size_t CScriptEditCtrl::GetSelectionAnchorX()  const { return m_iSelectionAnchorX; }
size_t CScriptEditCtrl::GetSelectionAnchorY()  const { return m_iSelectionAnchorY; }



void CScriptEditCtrl::SetPosition( int iX, int iY )
{

  m_iCursorX = iX;
  m_iCursorY = iY;
  ClipCoordinatesToText( &m_iCursorX, &m_iCursorY );
  ScrollIntoView();
  Invalidate();

}



void CScriptEditCtrl::SetSelection( size_t iX1, size_t iY1, size_t iX2, size_t iY2 )
{

  ClipCoordinatesToText( &iX2, &iY2 );
  ClipCoordinatesToText( &iX1, &iY1 );
  m_iCursorX = iX2;
  m_iCursorY = iY2;
  m_iSelectionAnchorX = iX1;
  m_iSelectionAnchorY = iY1;
  ScrollIntoView();
  Invalidate();

}



void CScriptEditCtrl::OnClose() 
{

	KillTimer( 1 );
	
	//CWnd::OnClose();

}



BOOL CScriptEditCtrl::FindString( GR::String strFindMe, size_t *iX, size_t *iY ) 
{

  for ( size_t i = 0; i < m_vectText.size(); i++ )
  {
    size_t iPos = m_vectText[i].find( strFindMe, 0 );
    if ( iPos != GR::String::npos )
    {
      // gefunden!
      *iX = iPos;
      *iY = i;
      return TRUE;
    }
  }
  *iX = 0;
  *iY = 0;
  return FALSE;

}




void CScriptEditCtrl::Cut()
{
  // Cut to Clipboard
  if ( !m_bSelection )          return;
  if ( m_vectText.size() <= 0 ) return;

  size_t             iX1,
                  iY1,
                  iX2,
                  iY2;
  GR::String     strDummy;

  if ( m_bSelection )
  {
    iX1 = m_iSelectionAnchorX;
    iY1 = m_iSelectionAnchorY;
    iX2 = m_iCursorX;
    iY2 = m_iCursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_iCursorX;
      iY1 = m_iCursorY;
      iX2 = m_iSelectionAnchorX;
      iY2 = m_iSelectionAnchorY;
    }
  }
  else
  {
    iX1 = 0;
    iY1 = 0;
    iX2 = m_vectText[m_vectText.size() - 1].length();
    iY2 = m_vectText.size() - 1;
  }
  if ( ( iX1 == iX2 )
  &&   ( iY1 == iY2 ) )
  {
    // nix da!
    if ( OpenClipboard() )
    {
      EmptyClipboard();
      CloseClipboard();
    }
    return;
  }
	if ( OpenClipboard() )
	{
    size_t   iChar = iX1,
              iZeile = iY1;

    strDummy = "";
    do
    {
      if ( iZeile < m_vectText.size() )
      {
        if ( iChar < m_vectText[iZeile].length() )
        {
          strDummy += m_vectText[iZeile][iChar];
        }
        iChar++;
        if ( ( iZeile < iY2 )
        ||   ( ( iZeile == iY2 )
        &&     ( iChar < iX2 ) ) )
        {
          if ( iChar >= m_vectText[iZeile].length() )
          {
            // Zeilenumbruch!
            strDummy += "\r\n";
            iChar = 0;
            iZeile++;
          }
        }
      }
      if ( ( iZeile == iY2 )
      &&   ( iChar >= iX2 ) )
      {
        break;
      }
    }
    while ( TRUE );

    strDummy += '\0';

		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strDummy.length() + 1 );
		buffer = (char*)GlobalLock( clipbuffer );
		strcpy( buffer, strDummy.c_str() );
		GlobalUnlock( clipbuffer );
		SetClipboardData( CF_TEXT, clipbuffer );
		CloseClipboard();

    if ( m_bSelection )
    {
      RemoveSelection();
    }
    else
    {
      Reset();
    }
    m_bModified = TRUE;
    AdjustScrollBars();
    Invalidate();
  }
}


GR::String CScriptEditCtrl::GetSelection()
{

  if ( m_vectText.empty() ) return GR::String();

  size_t             iX1, iY1, iX2, iY2;
  GR::String     strDummy;

  if ( m_bSelection )
  {
    iX1 = m_iSelectionAnchorX;
    iY1 = m_iSelectionAnchorY;
    iX2 = m_iCursorX;
    iY2 = m_iCursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_iCursorX;
      iY1 = m_iCursorY;
      iX2 = m_iSelectionAnchorX;
      iY2 = m_iSelectionAnchorY;
    }
  }
  else
  {
    return GR::String();
    // iX1 = 0;
    // iY1 = 0;
    // iX2 = m_vectText[m_vectText.size() - 1].length();
    // iY2 = m_vectText.size() - 1;
  }
  
  if ( ( iX1 == iX2 )  
  &&   ( iY1 == iY2 ) ) 
  {
    return GR::String();
  }

  size_t   iChar = iX1, iZeile = iY1;
  strDummy = "";
  do
  {
    if ( iZeile < m_vectText.size() )
    {
      if ( iChar < m_vectText[iZeile].length() )
      {
        strDummy += m_vectText[iZeile][iChar];
      }
      iChar++;
      if ( ( iZeile < iY2 )
      ||   ( ( iZeile == iY2 )
      &&     ( iChar < iX2 ) ) )
      {
        if ( iChar >= m_vectText[iZeile].length() )
        {
          // Zeilenumbruch!
          strDummy += "\r\n";
          iChar = 0;
          iZeile++;
        }
      }
    }
    if ( ( iZeile == iY2 )
    &&   ( iChar >= iX2 ) )
    {
      break;
    }
  }
  while ( TRUE );

  strDummy += '\0';


  return strDummy;
}



void CScriptEditCtrl::Copy()
{
  if ( !m_bSelection )
  {
    return;
  }
  if ( m_vectText.size() <= 0 )
  {
    return;
  }

  size_t             iX1,
                  iY1,
                  iX2,
                  iY2;
  GR::String     strDummy;

  if ( m_bSelection )
  {
    iX1 = m_iSelectionAnchorX;
    iY1 = m_iSelectionAnchorY;
    iX2 = m_iCursorX;
    iY2 = m_iCursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_iCursorX;
      iY1 = m_iCursorY;
      iX2 = m_iSelectionAnchorX;
      iY2 = m_iSelectionAnchorY;
    }
  }
  else
  {
    iX1 = 0;
    iY1 = 0;
    iX2 = m_vectText[m_vectText.size() - 1].length();
    iY2 = m_vectText.size() - 1;
  }
  if ( ( iX1 == iX2 )
  &&   ( iY1 == iY2 ) )
  {
    // nix da!
    if ( OpenClipboard() )
    {
      EmptyClipboard();
      CloseClipboard();
    }
    return;
  }
	if ( OpenClipboard() )
	{
    size_t   iChar = iX1,
             iZeile = iY1;

    strDummy = "";
    do
    {
      if ( iZeile < m_vectText.size() )
      {
        if ( iChar < m_vectText[iZeile].length() )
        {
          strDummy += m_vectText[iZeile][iChar];
        }
        iChar++;
        if ( ( iZeile < iY2 )
        ||   ( ( iZeile == iY2 )
        &&     ( iChar < iX2 ) ) )
        {
          if ( iChar >= m_vectText[iZeile].length() )
          {
            // Zeilenumbruch!
            strDummy += "\r\n";
            iChar = 0;
            iZeile++;
          }
        }
      }
      if ( ( iZeile == iY2 )
      &&   ( iChar >= iX2 ) )
      {
        break;
      }
    }
    while ( TRUE );

    strDummy += '\0';

		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strDummy.length() + 1 );
		buffer = (char*)GlobalLock( clipbuffer );
		strcpy( buffer, strDummy.c_str() );
		GlobalUnlock( clipbuffer );
		SetClipboardData( CF_TEXT, clipbuffer );
		CloseClipboard();
  }

}



void CScriptEditCtrl::Paste()
{
  // Paste from Clipboard
  if ( !IsClipboardFormatAvailable( CF_TEXT ) ) 
  {
    return;
  }
  
  if ( OpenClipboard() )
  {
    RemoveSelection();
    char        *cBuffer,
                cDummy;
    HANDLE      hData = GetClipboardData( CF_TEXT );

	  cBuffer = (char*)GlobalLock( hData );
    if ( cBuffer != NULL )
    {
      GR::String   strDummy = "";
      int           iPos = 0;
      while ( TRUE )
      {
        cDummy = cBuffer[iPos];
        if ( cDummy == 0 )
        {
          InsertString( strDummy, FALSE );
          break;
        }
        if ( cDummy == 10 )
        {
          // String mit Zeilenumbruch!
          std::vector<GR::String>::iterator    itVectString;
          size_t                                   iDummy = m_iCursorY;


          itVectString = m_vectText.begin();
          while ( iDummy )
          {
            itVectString++;
            iDummy--;
          }
          m_vectText.insert( itVectString, "" );
          if ( m_iCursorX == m_vectText[m_iCursorY + 1].length() )
          {
            m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1] + strDummy;
            m_iCursorY++;
            m_vectText[m_iCursorY] = "";
          }
          else
          {
            m_vectText[m_iCursorY] = m_vectText[m_iCursorY + 1].substr( 0, m_iCursorX ) + strDummy;
            m_iCursorY++;
            if ( m_vectText[m_iCursorY].length() > 0 )
            {
              m_vectText[m_iCursorY] = m_vectText[m_iCursorY].substr( m_iCursorX );
            }
            else
            {
              m_vectText[m_iCursorY] = "";
            }
            m_iCursorX = 0;
          }
          strDummy = "";
        }
        else if ( cDummy != 13 )
        {
          if ( m_dwFlags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
          {
            if ( cDummy == VK_TAB )
            {
              for ( DWORD i = 0; i < m_dwTabSpacing; i++ )
              {
                strDummy += ' ';
              }
              iPos++;
              continue;
            }
          }
          strDummy += cDummy;
        }
        iPos++;
      }
    }
    GlobalUnlock( hData );
    m_bModified = TRUE;
	  CloseClipboard();
    ScrollIntoView();
    Invalidate();
  }
}



void CScriptEditCtrl::FillSolidRect( HDC hdc, int iX, int iY, int iCX, int iCY, COLORREF color )
{

	::SetBkColor( hdc, color );

  RECT    rc;

  SetRect( &rc, iX, iY, iX + iCX, iY + iCY );

	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );

}



void CScriptEditCtrl::FillSolidRect( HDC hdc, RECT& rc, COLORREF color )
{

	::SetBkColor( hdc, color );

	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );

}



LRESULT CScriptEditCtrl::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_SETFOCUS:
      OnSetFocus( (HWND)wParam );
      break;
    case WM_KILLFOCUS:
      OnKillFocus( (HWND)wParam );
      break;
    case WM_TIMER:
      OnTimer( (UINT)wParam );
      break;
    case WM_CLOSE:
      OnClose();
      break;
    case WM_GETDLGCODE:
      return DLGC_WANTALLKEYS;
    case WM_SETCURSOR:
      if ( LOWORD( lParam ) == HTCLIENT )
      {
        SetCursor( LoadCursor( NULL, IDC_IBEAM ) );
        return 0;
      }
      break;
    case 0x020a: //WM_MOUSEWHEEL:
      {
        int   iDelta = ( (short)HIWORD( wParam ) ) / 120; //WHEEL_DELTA;

        while ( iDelta > 0 )
        {
          SendMessage( WM_VSCROLL, SB_LINEUP, 0 );
          --iDelta;
        }
        while ( iDelta < 0 )
        {
          SendMessage( WM_VSCROLL, SB_LINEDOWN, 0 );
          ++iDelta;
        }
      }
      break;
    case WM_VSCROLL:
      OnVScroll( LOWORD( wParam ), HIWORD( wParam ) );
      return 0;
    case WM_HSCROLL:
      OnHScroll( LOWORD( wParam ), HIWORD( wParam ) );
      return 0;
    case WM_ERASEBKGND:
      return TRUE;
    case WM_KEYDOWN:
      OnKeyDown( (UINT)wParam, (UINT)( lParam & 0x0f ), 0 );
      break;
    case WM_CHAR:
      OnChar( (UINT)wParam, (UINT)( lParam & 0x0f ), 0 );
      break;
    case WM_LBUTTONDBLCLK:
      {
        POINT   pt;
        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonDblClk( (UINT)wParam, pt );
      }
      break;
    case WM_LBUTTONUP:
      {
        POINT   pt;
        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonUp( (UINT)wParam, pt );
      }
      break;
    case WM_LBUTTONDOWN:
      {
        POINT   pt;
        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonDown( (UINT)wParam, pt );
      }
      break;
    case WM_MOUSEMOVE:
      {
        POINT   pt;
        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnMouseMove( (UINT)wParam, pt );
      }
      break;
    case WM_PAINT:
      OnPaint();
      return TRUE;
  }

  return CWnd::WindowProc( uMsg, wParam, lParam );

}



void CScriptEditCtrl::AppendText( const GR::string& strText )
{

  // nach ganz unten
  SetSelection( m_iCursorX, m_iCursorY, m_iCursorX, m_iCursorY );
  size_t   iOldX = m_iCursorX;
  size_t   iOldY = m_iCursorY;

  m_iCursorX = m_vectText[m_vectText.size() - 1].length();
  m_iCursorY = m_vectText.size() - 1;

  InsertString( strText );

  if ( ::GetFocus() == m_hWnd )
  {
    m_iCursorX = iOldX;
    m_iCursorY = iOldY;
  }
  ScrollIntoView();
  Invalidate();

}



