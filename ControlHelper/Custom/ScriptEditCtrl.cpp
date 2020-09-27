// ScriptEditCtrl.cpp: Implementierungsdatei
//

#include <ControlHelper/Custom/ScriptEditCtrl.h>


BOOL CScriptEditCtrl::m_Registered = Register();		// Register the control during class initialization



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
	wc.lpszClassName = "GRScriptEdit";				// Class name
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
                      DEFAULT_PITCH, "Courier New" );

  m_Focus                  = FALSE;
  m_Selection              = FALSE;
  m_SelectingByMouse       = FALSE;

  m_Modified               = FALSE;

  m_CharWidth              = 9;
  m_CharHeight             = 17;
  m_VisibleCharsPerLine    = 0;
  m_VisibleLines           = 0;

  m_TabSpacing            = 2;
  m_Flags                 = SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS;

  m_vectColor.push_back( GetSysColor( COLOR_WINDOW ) );
  m_vectColor.push_back( GetSysColor( COLOR_WINDOWTEXT ) );
  m_vectColor.push_back( GetSysColor( COLOR_HIGHLIGHT ) );
  m_vectColor.push_back( GetSysColor( COLOR_HIGHLIGHTTEXT ) );
  m_vectColor.push_back( GetSysColor( COLOR_INACTIVEBORDER ) );
  m_vectColor.push_back( GetSysColor( COLOR_GRAYTEXT ) );
  m_vectColor.push_back( RGB( 150, 50, 50 ) );
  m_vectColor.push_back( RGB( 50, 150, 50 ) );

  m_Enabled                = TRUE;

  Reset();

  // Keyword-Liste löschen
  m_SyntaxEntry.clear();
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

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, "GRScriptEdit", NULL,
                        WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, hwndParent, iID ) )
  {
    return FALSE;
  }
  SetScrollRange( SB_HORZ, 0, SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH );
  SetScrollRange( SB_VERT, 0, 0 );

  GetClientRect( &m_rectRC );
  m_VisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_CharWidth;
  m_VisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_CharHeight;

  CreateSolidCaret( 1, m_CharHeight );
  m_ShowCaret = FALSE;

  m_Flags = dwFlags;

  SetTimer( 1, 100, NULL );

  EnableWindow();

  return TRUE;
}



void CScriptEditCtrl::ScrollToBottom()
{
  SetScrollPos( SB_VERT, (int)m_Text.size() - m_VisibleLines );
}



void CScriptEditCtrl::Reset()
{
  m_FirstLine              = 0;
  m_FirstCharacter         = 0;
  m_CursorWantedX          = -1;
  m_CursorX                = 0;
  m_CursorY                = 0;
  m_CaretX                 = 0;

  m_SelectionAnchorX       = 0;
  m_SelectionAnchorY       = 0;

  // Text-Vektor initialisieren (sonst gibt's HaufenvieleErrors)
  m_Text.clear();
  m_Text.push_back( "" );

  m_Modified               = FALSE;

  AdjustScrollBars();
  Invalidate();

}



BOOL CScriptEditCtrl::SetWindowPos( HWND hwndAfter, int x, int y, int cx, int cy, UINT nFlags )
{

  if ( !IsWindow() )
  {
    return FALSE;
  }
  BOOL bResult = CWnd::SetWindowPos( hwndAfter, x, y, cx, cy, nFlags );

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


  hFile = CreateFile( szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  if ( hFile == INVALID_HANDLE_VALUE )
  {
    return FALSE;
  }
  do
  {
    char            a         = '\0';

    unsigned long   ulOffset  = 0,
                    ulBytesRead;
    GR::String      strDummy  = "";

    while ( TRUE )
    {
      ReadFile( hFile, &a, 1, &ulBytesRead, NULL );
      if ( ulBytesRead == 0 )
      {
        m_Text.push_back( strDummy );
        CloseHandle( hFile );
        hFile = INVALID_HANDLE_VALUE;
        break;
      }
      if ( ( ulBytesRead == 0 )
      ||   ( a == 10 ) )
      {
        m_Text.push_back( strDummy );
        break;
      }
      else if ( a != 13 )
      {
        if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
        {
          if ( a == VK_TAB )
          {
            for ( DWORD i = 0; i < m_TabSpacing; i++ )
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

  m_Modified = FALSE;
  AdjustScrollBars();
  ScrollIntoView();
  UpdateSelectionAnchor( TRUE );
  Invalidate();
  return TRUE;

}



void CScriptEditCtrl::SaveToFile( const char *szFileName )
{

  HANDLE        hFile;


  hFile  = CreateFile( szFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  if ( hFile == INVALID_HANDLE_VALUE )
  {
    return;
  }
  DWORD     dwBytesWritten;

  char      caNL[2] = { 13, 10 };

  for ( size_t i = 0; i < m_Text.size(); i++ )
  {
    // Zeile
    WriteFile( hFile, m_Text[i].c_str(), (DWORD)m_Text[i].length(), &dwBytesWritten, NULL );
    // #13#10
    WriteFile( hFile, caNL, 2, &dwBytesWritten, NULL );
  }

  CloseHandle( hFile );

}



GR::String CScriptEditCtrl::GetLine( DWORD dwLine )
{
  GR::String     strResult = "";

  if ( dwLine < m_Text.size() )
  {
    strResult = m_Text[dwLine];
  }
  return strResult;
}



DWORD CScriptEditCtrl::GetLineCount()
{
  return (DWORD)m_Text.size();
}



void CScriptEditCtrl::GetWindowText( GR::String& strText ) const
{
  strText = "";
  for ( size_t i = 0; i < m_Text.size(); i++ )
  {
    strText += m_Text[i].c_str();
    strText += "\r\n";
  }
}



void CScriptEditCtrl::OnPaint()
{
  if ( m_Text.size() == 0 )
  {
    return;
  }
  DrawCursor();


  HDC   hdcOld;

	PAINTSTRUCT ps;

  hdcOld = BeginPaint( m_hWnd, &ps );

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
	long x;
	long y = 0;
  GR::String     temp;

  GetClientRect( &m_rectRC );
  m_VisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_CharWidth;
  m_VisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_CharHeight;
  if ( m_Enabled )
  {
    FillSolidRect( hdc, m_rectRC, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
  }
  else
  {
    FillSolidRect( hdc, m_rectRC, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
  }
  for ( int p = m_FirstLine; p < m_FirstLine + m_VisibleLines + 1; p++ )
	{
    if ( p >= (int)m_Text.size() )
    {
      break;
    }
    if ( m_Text[p].length() == 0 )
    {
      y += m_CharHeight;
      continue;
    }

    std::vector<COLORREF>   vectColors;

    vectColors.resize( m_Text[p].length() + 2 );
    FillColorVector( m_Text[p], p, &vectColors );

    x = 0;
		temp = m_Text[p];

    if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
    {
		  for ( size_t j = m_FirstCharacter; j < temp.length(); j++ )
		  {
        if ( IsInsideSelection( (int)j, p ) )
        {
          FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
          SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
        }
        else
        {
          if ( m_Enabled )
          {
            SetTextColor( hdc, vectColors[j] );
            FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
          }
          else
          {
            SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
            FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
          }
        }
        TextOut( hdc, x, y, temp.substr( j, 1 ).c_str(), 1 );
        x += m_CharWidth;
		  }
    }
    else
    {
      int     iRealX,
              iLeftSpaces,
              iPos;
      iRealX = 0;
      iLeftSpaces = 0;

      iPos = 0;

      do
		  {
        if ( iRealX > m_FirstCharacter + m_VisibleCharsPerLine )
        {
          break;
        }
        if ( iLeftSpaces )
        {
          iLeftSpaces--;
          if ( iRealX >= m_FirstCharacter )
          {
            if ( IsInsideSelection( iPos, p ) )
            {
              FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
              SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
            }
            else
            {
              if ( m_Enabled )
              {
                SetTextColor( hdc, vectColors[iPos] );
                FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
              }
              else
              {
                SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
                FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
              }
            }

			      TextOut( hdc, x, y, " ", 1 );
            x += m_CharWidth;
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
            iLeftSpaces = m_TabSpacing;
          }
          else
          {
            if ( iRealX >= m_FirstCharacter )
            {
              if ( IsInsideSelection( iPos, p ) )
              {
                FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_BACKGROUND] );
                SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::SELECTED_TEXT] );
              }
              else
              {
                if ( m_Enabled )
                {
                  SetTextColor( hdc, vectColors[iPos] );
                  FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::BACKGROUND] );
                }
                else
                {
                  SetTextColor( hdc, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_TEXT] );
                  FillSolidRect( hdc, x, y, m_CharWidth, m_CharHeight, m_vectColor[SCRIPT_EDIT_CONTROL::COLOR::DISABLED_BACKGROUND] );
                }
              }

              TextOut( hdc, x, y, temp.substr( iPos, 1 ).c_str(), 1 );
              x += m_CharWidth;
            }
            iPos++;
            iRealX++;
          }
        }
		  }
      while ( iPos < (int)temp.length() );
    }
		y += m_CharHeight;
	}

  // Doppelpuffer zurückkopieren
  BitBlt( hdcOld, rectMem.left, rectMem.top, rectMem.right - rectMem.left, rectMem.bottom - rectMem.top,
		              hdc, rectMem.left, rectMem.top, SRCCOPY );
	SelectObject( hdc, m_oldBitmap );
  DeleteObject( m_bitmapMem );

  SelectObject( hdc, hOldFont );

  DeleteDC( hdc );

	EndPaint( m_hWnd, &ps );
}



BOOL CScriptEditCtrl::IsAbove( int iX1, int iY1, int iX2, int iY2 )
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



BOOL CScriptEditCtrl::IsInsideSelection( int iX, int iY )
{
  if ( !m_Selection )
  {
    return FALSE;
  }
  int     iSelectionStartX    = m_SelectionAnchorX,
          iSelectionStartY    = m_SelectionAnchorY,
          iSelectionEndX      = m_CursorX,
          iSelectionEndY      = m_CursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_CursorX;
    iSelectionStartY    = m_CursorY;
    iSelectionEndX      = m_SelectionAnchorX;
    iSelectionEndY      = m_SelectionAnchorY;
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



std::vector<GR::String>::iterator CScriptEditCtrl::GetStringIteratorAt( int iY )
{
  if ( ( iY < 0 )
  ||   ( m_Text.size() == 0 )
  ||   ( iY >= (int)m_Text.size() ) )
  {
    return m_Text.end();
  }

  std::vector<GR::String>::iterator      itDummy = m_Text.begin();
  int   iPos = 0;
  while ( iPos < iY )
  {
    iPos++;
    itDummy++;
  }
  return itDummy;
}



DWORD CScriptEditCtrl::GetMaxLength()
{
  if ( m_Text.size() == 0 )
  {
    return 0;
  }
  DWORD     dwLength = 0,
            dwTrueLength = 0;
  for ( size_t i = 0; i < m_Text.size(); i++ )
  {
    dwTrueLength = GetTrueLength( m_Text[i] );
    if ( dwTrueLength > dwLength )
    {
      dwLength = dwTrueLength;
    }
  }
  return dwLength;
}



DWORD CScriptEditCtrl::GetTrueLength( const GR::String& strLine )
{
  if ( m_Text.size() == 0 )
  {
    return 0;
  }
  if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return (DWORD)strLine.length();
  }
  DWORD     dwPos = 0,
            dwTrueLength = 0;
  do
  {
    if ( strLine[dwPos] == VK_TAB )
    {
      dwTrueLength += m_TabSpacing;
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

  if ( m_Text.size() == 0 )
  {
    return 0;
  }
  if ( iTrueX == 0 )
  {
    return 0;
  }
  if ( iY >= (int)m_Text.size() )
  {
    return 0;
  }
  if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return iTrueX;
  }
  int       iDummy = 0;
  DWORD     dwPos = 0;
  while ( iDummy < iTrueX )
  {
    if ( m_Text[iY][dwPos] == VK_TAB )
    {
      iDummy += m_TabSpacing;
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

  if ( m_Text.size() == 0 )
  {
    return 0;
  }
  if ( iX == 0 )
  {
    return 0;
  }
  if ( iY >= (int)m_Text.size() )
  {
    return 0;
  }
  if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    return iX;
  }
  int       iDummy = 0;
  DWORD     dwPos = 0;
  while ( iDummy < iX )
  {
    if ( m_Text[iY][dwPos] == VK_TAB )
    {
      dwPos += m_TabSpacing;
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

  int     iSelectionStartX    = m_SelectionAnchorX,
          iSelectionStartY    = m_SelectionAnchorY,
          iSelectionEndX      = m_CursorX,
          iSelectionEndY      = m_CursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_CursorX;
    iSelectionStartY    = m_CursorY;
    iSelectionEndX      = m_SelectionAnchorX;
    iSelectionEndY      = m_SelectionAnchorY;
  }

  int   iAnchorX = m_SelectionAnchorX,
        iAnchorY = m_SelectionAnchorY;

  RemoveSelection();
  InsertString( strNew, TRUE, bSelect );

  if ( bSelect )
  {
  m_Selection = TRUE;
  m_SelectionAnchorX = iSelectionStartX;
  m_SelectionAnchorY = iSelectionStartY;
  }
  else
  {
    m_Selection = FALSE;

  }
  m_Modified = TRUE;

}



void CScriptEditCtrl::RemoveSelection()
{

  if ( !m_Enabled )
  {
    return;
  }
  if ( !m_Selection )
  {
    return;
  }
  int     iSelectionStartX    = m_SelectionAnchorX,
          iSelectionStartY    = m_SelectionAnchorY,
          iSelectionEndX      = m_CursorX,
          iSelectionEndY      = m_CursorY;


  if ( !IsAbove( iSelectionStartX, iSelectionStartY, iSelectionEndX, iSelectionEndY ) )
  {
    // Koordinaten tauschen
    iSelectionStartX    = m_CursorX;
    iSelectionStartY    = m_CursorY;
    iSelectionEndX      = m_SelectionAnchorX;
    iSelectionEndY      = m_SelectionAnchorY;
  }
  if ( iSelectionStartY == iSelectionEndY )
  {
    // Spezialfall, Selection innerhalb einer Zeile
    m_Text[iSelectionStartY].erase( iSelectionStartX, iSelectionEndX - iSelectionStartX );
    m_CursorX = iSelectionStartX;
  }
  else
  {
    // Anfang der Selection rausnehmen
    m_Text[iSelectionStartY] = m_Text[iSelectionStartY].substr( 0, iSelectionStartX );
    // Endteil der Selection rausnehmen
    m_Text[iSelectionEndY] = m_Text[iSelectionEndY].substr( iSelectionEndX );

    m_Text[iSelectionStartY] += m_Text[iSelectionEndY];

    std::vector<GR::String>::iterator      itSelEnd = GetStringIteratorAt( iSelectionEndY );
    m_Text.erase( itSelEnd );

    // dazwischenliegende Zeilen entfernen
    if ( iSelectionEndY - iSelectionStartY >= 2 )
    {
      std::vector<GR::String>::iterator    itBegin = GetStringIteratorAt( iSelectionStartY + 1 ),
                                            itEnd   = GetStringIteratorAt( iSelectionEndY );
      m_Text.erase( itBegin, itEnd );
    }
    m_CursorX = iSelectionStartX;
    m_CursorY = iSelectionStartY;
  }
  m_SelectionAnchorX = m_CursorX;
  m_SelectionAnchorY = m_CursorY;
  m_Modified = TRUE;
  m_Selection = FALSE;

}



void CScriptEditCtrl::InsertChar( char cChar, BOOL bRedraw )
{
  GR::String     strDummy = "";

  strDummy += cChar;

  InsertString( strDummy, bRedraw );
}



void CScriptEditCtrl::InsertString( const GR::String& strMyNew, BOOL bRedraw, BOOL bSelect )
{
  if ( !m_Enabled )
  {
    return;
  }

  if ( m_CursorY > (int)m_Text.size() + 1 )
  {
    return;
  }
  if ( ( m_Text.size() == 0 )
  &&   ( m_CursorY == 0 ) )
  {
    // diesen String gibt es (noch) gar nicht
    m_Text.push_back( strMyNew );
  }
  else
  {
    GR::String     strResult = "";

    GR::String     strNew = strMyNew;


    if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
    {
      GR::String   strDummy = "";
      for ( size_t i = 0; i < strNew.length(); i++ )
      {
        if ( strNew[i] == VK_TAB )
        {
          for ( DWORD j = 0; j < m_TabSpacing; j++ )
          {
            strDummy += ' ';
          }
        }
        else
        {
          strDummy += strNew[i];
        }
      }
      GR::String    temp = strDummy;
      strDummy = strNew;
      strNew = temp;
    }
    for ( size_t i = 0; i < strNew.length(); i++ )
    {
      if ( strNew[i] == 10 )
      {
        // ein Zeilenumbruch ist da drin!
        if ( m_CursorX < (int)m_Text[m_CursorY].length() )
        {
          // Zeilenumbruch einfügen
          std::vector<GR::String>::iterator    itVectString;
          int                                   iDummy = m_CursorY;
          GR::String     strDummy = "";


          itVectString = m_Text.begin();
          while ( iDummy )
          {
            itVectString++;
            iDummy--;
          }
          m_Text.insert( itVectString, "" );
          m_Text[m_CursorY] = m_Text[m_CursorY + 1].substr( 0, m_CursorX );
          m_CursorY++;
          m_Text[m_CursorY] = m_Text[m_CursorY].substr( m_CursorX );
          m_CursorX = 0;
        }
        else
        {
          // Zeilenumbruch einfügen
          std::vector<GR::String>::iterator    itVectString;
          GR::String     strDummy = "";


          itVectString = m_Text.begin() + m_CursorY;
          m_Text.insert( itVectString, "" );
          m_Text[m_CursorY] = m_Text[m_CursorY + 1].substr( 0, m_CursorX );
          m_CursorY++;
          m_Text[m_CursorY] = "";
          m_CursorX = 0;
        }
      }
      else if ( ( (BYTE)strNew[i] >= 32 )
      ||        ( (BYTE)strNew[i] == VK_TAB ) )
      {
        // Zeichen einfügen
        strResult = "";
        if ( m_CursorX > 0 )
        {
          strResult = m_Text[m_CursorY].substr( 0, m_CursorX );
        }
        strResult += (char)strNew[i];
        if ( m_CursorX < (int)m_Text[m_CursorY].length() )
        {
          strResult += m_Text[m_CursorY].substr( m_CursorX );
        }
        m_Text[m_CursorY] = strResult;
        m_CursorX++;
      }
    }
  }
  m_Modified = TRUE;
  AdjustScrollBars();

  UpdateSelectionAnchor( bSelect );
  Invalidate();
}



void CScriptEditCtrl::UpdateCaret()
{
  if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    m_CaretX = m_CursorX;
  }
  else
  {
    m_CaretX = 0;
    for ( int i = 0; i < m_CursorX; i++ )
    {
      if ( m_Text[m_CursorY].at( i ) == VK_TAB )
      {
        m_CaretX += m_TabSpacing;
      }
      else
      {
        m_CaretX++;
      }
    }
  }

}



void CScriptEditCtrl::DrawCursor()
{
  UpdateCaret();
  if ( m_Focus )
  {
    if ( !m_ShowCaret )
    {
      ShowCaret();
      m_ShowCaret = TRUE;
    }
    POINT   ptCaret;

    ptCaret.x = ( m_CaretX - m_FirstCharacter ) * m_CharWidth;
    ptCaret.y = ( m_CursorY - m_FirstLine ) * m_CharHeight;
    SetCaretPos( ptCaret );
  }
}



void CScriptEditCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
  if ( m_SelectingByMouse )
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
    if ( ( bControlPressed )
    &&   ( m_Enabled ) )
    {
      Paste();
    }
    return;
  }
  else if ( nChar == 'C' )
  {
    if ( bControlPressed )
    {
      Copy();
    }
    return;
  }
  else if ( nChar == 'X' )
  {
    if ( ( bControlPressed )
    &&   ( m_Enabled ) )
    {
      Cut();
    }
    return;
  }
  else if ( nChar == 'A' )
  {
    if ( ( bControlPressed )
    &&   ( m_Enabled ) )
    {
      if ( m_Text.empty() )
      {
        SetSelection( 0, 0, 0, 0 );
        m_Selection = false;
      }
      else
      {
        SetSelection( 0, 0, m_Text[m_Text.size() - 1].length(), (int)m_Text.size() - 1 );
        m_Selection = true;
      }
    }
    return;
  }
  else
    // {
    // if ( bControlPressed )
    // {
      // if ( !m_Enabled )
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
              // int                                   iDummy = m_CursorY;


              // itVectString = m_Text.begin();
              // while ( iDummy )
              // {
                // itVectString++;
                // iDummy--;
              // }
              // m_Text.insert( itVectString );
              // if ( m_CursorX == m_Text[m_CursorY + 1].length() )
              // {
                // m_Text[m_CursorY] = m_Text[m_CursorY + 1] + strDummy;
                // m_CursorY++;
                // m_Text[m_CursorY] = "";
              // }
              // else
              // {
                // m_Text[m_CursorY] = m_Text[m_CursorY + 1].substr( 0, m_CursorX ) + strDummy;
                // m_CursorY++;
                // if ( m_Text[m_CursorY].length() > 0 )
                // {
                  // m_Text[m_CursorY] = m_Text[m_CursorY].substr( m_CursorX );
                // }
                // else
                // {
                  // m_Text[m_CursorY] = "";
                // }
                // m_CursorX = 0;
              // }
              // strDummy = "";
            // }
            // else if ( cDummy != 13 )
            // {
              // if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
              // {
                // if ( cDummy == VK_TAB )
                // {
                  // for ( DWORD i = 0; i < m_TabSpacing; i++ )
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
        // m_Modified = TRUE;
// 	      CloseClipboard();
        // ScrollIntoView();
        // Invalidate();
        // return;
// 	    }
    // }
  // }
  // else if ( nChar == 'C' )
  // {
    // if ( bControlPressed && m_Selection )
    // {
      // // Copy to Clipboard
      // if ( m_Text.size() > 0 )
      // {
        // int             iX1,
                        // iY1,
                        // iX2,
                        // iY2;
        // GR::String     strDummy;

        // if ( m_Selection )
        // {
          // iX1 = m_SelectionAnchorX;
          // iY1 = m_SelectionAnchorY;
          // iX2 = m_CursorX;
          // iY2 = m_CursorY;
          // if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
          // {
            // // Koordinaten tauschen
            // iX1 = m_CursorX;
            // iY1 = m_CursorY;
            // iX2 = m_SelectionAnchorX;
            // iY2 = m_SelectionAnchorY;
          // }
        // }
        // else
        // {
          // iX1 = 0;
          // iY1 = 0;
          // iX2 = m_Text[m_Text.size() - 1].length();
          // iY2 = m_Text.size() - 1;
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
            // if ( iZeile < m_Text.size() )
            // {
              // if ( iChar < m_Text[iZeile].length() )
              // {
                // strDummy += m_Text[iZeile][iChar];
              // }
              // iChar++;
              // if ( ( iZeile < iY2 )
              // ||   ( ( iZeile == iY2 )
              // &&     ( iChar < iX2 ) ) )
              // {
                // if ( iChar >= m_Text[iZeile].length() )
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
    // if ( bControlPressed && m_Selection )
    // {
      // // Cut to Clipboard
      // if ( m_Text.size() > 0 )
      // {
        // int             iX1,
                        // iY1,
                        // iX2,
                        // iY2;
        // GR::String     strDummy;

        // if ( m_Selection )
        // {
          // iX1 = m_SelectionAnchorX;
          // iY1 = m_SelectionAnchorY;
          // iX2 = m_CursorX;
          // iY2 = m_CursorY;
          // if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
          // {
            // // Koordinaten tauschen
            // iX1 = m_CursorX;
            // iY1 = m_CursorY;
            // iX2 = m_SelectionAnchorX;
            // iY2 = m_SelectionAnchorY;
          // }
        // }
        // else
        // {
          // iX1 = 0;
          // iY1 = 0;
          // iX2 = m_Text[m_Text.size() - 1].length();
          // iY2 = m_Text.size() - 1;
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
            // if ( iZeile < m_Text.size() )
            // {
              // if ( iChar < m_Text[iZeile].length() )
              // {
                // strDummy += m_Text[iZeile][iChar];
              // }
              // iChar++;
              // if ( ( iZeile < iY2 )
              // ||   ( ( iZeile == iY2 )
              // &&     ( iChar < iX2 ) ) )
              // {
                // if ( iChar >= m_Text[iZeile].length() )
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

          // if ( m_Selection )
          // {
            // RemoveSelection();
          // }
          // else
          // {
            // Reset();
          // }
          // m_Modified = TRUE;
          // AdjustScrollBars();
          // Invalidate();
        // }
      // }
    // }

  // }
  if ( nChar == 8 )
  {
    // BackSpace!
    if ( !m_Enabled )
    {
      return;
    }
    if ( m_Selection )
    {
      RemoveSelection();
      AdjustScrollBars();
    }
    else
    {
      if ( m_CursorX > 0 )
      {
        m_Text[m_CursorY].erase( m_CursorX - 1, 1 );
        m_CursorX--;

        AdjustScrollBars();
        ScrollIntoView();
        Invalidate();
      }
      else
      {
        // Zeilen zusammenlegen?
        if ( m_CursorY > 0 )
        {
          m_CursorY--;
          m_CursorX = (int)m_Text[m_CursorY].length();
          m_Text[m_CursorY] += m_Text[m_CursorY + 1];

          std::vector<GR::String>::iterator      itSelDummy = GetStringIteratorAt( m_CursorY + 1 );
          m_Text.erase( itSelDummy );

          AdjustScrollBars();
          ScrollIntoView();
          Invalidate();
        }
      }
    }
    m_Modified = TRUE;
  }
  if ( nChar == VK_LEFT )
  {
    if ( bControlPressed )
    {
      // Word-Hüpf
      if ( m_CursorX == 0 )
      {
        if ( m_CursorY > 0 )
        {
          // zum Ende der vorherigen Zeile
          m_CursorWantedX = -1;
          m_CursorY--;
          m_CursorX = (int)m_Text[m_CursorY].length();

          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_Selection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
      else
      {
        int     iPos = m_CursorX;
        BOOL    bAlphaCharFound = FALSE;
        if ( iPos >= (int)m_Text[m_CursorY].length() )
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
          if ( isalnum( m_Text[m_CursorY][iPos] ) )
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
        m_CursorX = iPos + 1;
        m_CursorWantedX = -1;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_Selection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
    }
    else
    {
      if ( m_CursorX > 0 )
      {
        m_CursorWantedX = -1;
        m_CursorX--;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_Selection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
      else
      {
        if ( m_CursorY > 0 )
        {
          m_CursorWantedX = -1;
          m_CursorY--;
          m_CursorX = (int)m_Text[m_CursorY].length();
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_Selection ) )
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
      if ( m_CursorX >= (int)m_Text[m_CursorY].length() )
      {
        if ( m_CursorY + 1 < (int)m_Text.size() )
        {
          // zum Anfang der nächsten Zeile
          m_CursorWantedX = -1;
          m_CursorY++;
          m_CursorX = 0;
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_Selection ) )
          {
            Invalidate();
          }
          DrawCursor();
        }
      }
      else
      {
        int     iPos = m_CursorX;
        BOOL    bOutofAlphaChar = FALSE;
        do
        {
          iPos++;
          if ( iPos >= (int)m_Text[m_CursorY].length() )
          {
            m_CursorX = (int)m_Text[m_CursorY].length();
            break;
          }
          if ( isalnum( m_Text[m_CursorY][iPos] ) )
          {
            if ( bOutofAlphaChar )
            {
              m_CursorX = iPos;
              break;
            }
          }
          else
          {
            bOutofAlphaChar = TRUE;
          }
        }
        while ( TRUE );
        m_CursorWantedX = -1;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_Selection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
    }
    else
    {
      if ( m_CursorX < (int)m_Text[m_CursorY].length() )
      {
        m_CursorWantedX = -1;
        m_CursorX++;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_Selection ) )
        {
          Invalidate();
        }
        DrawCursor();
      }
      else
      {
        if ( m_CursorY + 1 < (int)m_Text.size() )
        {
          m_CursorWantedX = -1;
          m_CursorY++;
          m_CursorX = 0;
          UpdateSelectionAnchor( TRUE );
          if ( ( ScrollIntoView() )
          ||   ( m_Selection ) )
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
    if ( m_CursorY > 0 )
    {
      m_CursorY--;
      if ( m_CursorWantedX == -1 )
      {
        m_CursorWantedX = m_CursorX;
      }
      else
      {
        m_CursorX = m_CursorWantedX;
      }
      if ( m_CursorX > (int)m_Text[m_CursorY].length() )
      {
        m_CursorX = (int)m_Text[m_CursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_Selection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == VK_DOWN )
  {
    if ( m_CursorY + 1 < (int)m_Text.size() )
    {
      m_CursorY++;
      if ( m_CursorWantedX == -1 )
      {
        m_CursorWantedX = m_CursorX;
      }
      else
      {
        m_CursorX = m_CursorWantedX;
      }
      if ( m_CursorX > (int)m_Text[m_CursorY].length() )
      {
        m_CursorX = (int)m_Text[m_CursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_Selection ) )
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
      m_CursorY -= m_VisibleLines * 3 / 4;
      if ( m_CursorY < 0 )
      {
        m_CursorY = 0;
      }
      if ( m_CursorWantedX == -1 )
      {
        m_CursorWantedX = m_CursorX;
      }
      else
      {
        m_CursorX = m_CursorWantedX;
      }
      if ( m_CursorX > (int)m_Text[m_CursorY].length() )
      {
        m_CursorX = (int)m_Text[m_CursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_Selection ) )
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
      m_CursorY += m_VisibleLines * 3 / 4;
      if ( m_CursorY >= (int)m_Text.size() )
      {
        m_CursorY = (int)m_Text.size() - 1;
      }
      if ( m_CursorWantedX == -1 )
      {
        m_CursorWantedX = m_CursorX;
      }
      else
      {
        m_CursorX = m_CursorWantedX;
      }
      if ( m_CursorX > (int)m_Text[m_CursorY].length() )
      {
        m_CursorX = (int)m_Text[m_CursorY].length();
      }
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_Selection ) )
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
      m_CursorX = 0;
      m_CursorY = 0;
      ScrollIntoView();
      UpdateSelectionAnchor( TRUE );
      Invalidate();
    }
    else
    {
      if ( m_CursorX > 0 )
      {
        m_CursorWantedX = -1;
        m_CursorX = 0;
        UpdateSelectionAnchor( TRUE );
        if ( ( ScrollIntoView() )
        ||   ( m_Selection ) )
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
      if ( m_Text.size() > 0 )
      {
        m_CursorX = (int)m_Text[m_Text.size() - 1].length();
        m_CursorY = (int)m_Text.size() - 1;
        ScrollIntoView();
        UpdateSelectionAnchor( TRUE );
        Invalidate();
      }
    }
    else
    {
      m_CursorWantedX = -1;
      m_CursorX = (int)m_Text[m_CursorY].length();
      UpdateSelectionAnchor( TRUE );
      if ( ( ScrollIntoView() )
      ||   ( m_Selection ) )
      {
        Invalidate();
      }
      DrawCursor();
    }
  }
  else if ( nChar == VK_RETURN )
  {
    if ( !m_Enabled )
    {
      if ( m_CursorY + 1 < (int)m_Text.size() )
      {
        m_CursorY++;
        m_CursorX = 0;
      }
      m_Modified = TRUE;
      AdjustScrollBars();
      ScrollIntoView();
      UpdateSelectionAnchor( TRUE );
      Invalidate();
      return;
    }
    m_Modified = TRUE;
    RemoveSelection();
    if ( m_CursorX < (int)m_Text[m_CursorY].length() )
    {
      // Zeilenumbruch einfügen
      std::vector<GR::String>::iterator    itVectString;
      int                                   iDummy = m_CursorY;
      GR::String     strDummy = "";


      itVectString = m_Text.begin();
      while ( iDummy )
      {
        itVectString++;
        iDummy--;
      }
      m_Text.insert( itVectString, "" );
      m_Text[m_CursorY] = m_Text[m_CursorY + 1].substr( 0, m_CursorX );
      m_CursorY++;
      m_Text[m_CursorY] = m_Text[m_CursorY].substr( m_CursorX );

      int     iIndent = 0;
      if ( m_Text[m_CursorY - 1].length() > 0 )
      {
        while ( m_Text[m_CursorY - 1].at( iIndent ) == ' ' )
        {
          iIndent++;
          if ( iIndent >= (int)m_Text[m_CursorY - 1].length() )
          {
            break;
          }
        }
        for ( int i = 0; i < iIndent; i++ )
        {
          m_Text[m_CursorY] = ' ' + m_Text[m_CursorY];
        }
      }

      m_CursorX = iIndent;
    }
    else
    {
      // Leerzeile einfügen
      std::vector<GR::String>::iterator    itVectString;
      int                                   iDummy = m_CursorY;
      GR::String     strDummy = "";

      itVectString = m_Text.begin();
      while ( iDummy )
      {
        itVectString++;
        iDummy--;
      }
      m_Text.insert( itVectString, "" );
      m_Text[m_CursorY] = m_Text[m_CursorY + 1];
      m_CursorY++;
      m_Text[m_CursorY] = "";


      int     iIndent = 0;
      if ( m_Text[m_CursorY - 1].length() > 0 )
      {
        while ( m_Text[m_CursorY - 1].at( iIndent ) == ' ' )
        {
          iIndent++;
          if ( iIndent >= (int)m_Text[m_CursorY - 1].length() )
          {
            break;
          }
        }
        for ( int i = 0; i < iIndent; i++ )
        {
          m_Text[m_CursorY] = ' ' + m_Text[m_CursorY];
        }
      }

      m_CursorX = iIndent;

    }
    AdjustScrollBars();
    ScrollIntoView();
    UpdateSelectionAnchor( TRUE );
    Invalidate();
  }
  else if ( nChar == VK_DELETE )
  {
    // Delete
    if ( !m_Enabled )
    {
      return;
    }
    m_Modified = TRUE;
    if ( m_Selection )
    {
      RemoveSelection();
    }
    else
    {
      if ( m_CursorX < (int)m_Text[m_CursorY].length() )
      {
        // ein Zeichen löschen
        m_Text[m_CursorY].erase( m_CursorX, 1 );
      }
      else
      {
        // Zeilen zusammenlegen?
        if ( m_CursorY + 1 < (int)m_Text.size() )
        {
          m_Text[m_CursorY] += m_Text[m_CursorY + 1];

          std::vector<GR::String>::iterator      itSelDummy = GetStringIteratorAt( m_CursorY + 1 );
          m_Text.erase( itSelDummy );
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


  ClipCoordinatesToText( &m_CursorX, &m_CursorY );
  if ( (int)m_Text.size() < m_VisibleLines )
  {
    bScrollVert = FALSE;
  }

  UpdateCaret();

  while ( m_CaretX < m_FirstCharacter )
  {
    m_FirstCharacter--;
    if ( m_FirstCharacter < 0 )
    {
      m_FirstCharacter = 0;
    }
    bHorzChanged = TRUE;
  }
  while ( m_CaretX > m_FirstCharacter + m_VisibleCharsPerLine )
  {
    m_FirstCharacter++;
    bHorzChanged = TRUE;
  }
  /*
  while ( m_CursorX < m_FirstCharacter )
  {
    m_FirstCharacter--;
    if ( m_FirstCharacter < 0 )
    {
      m_FirstCharacter = 0;
    }
    bHorzChanged = TRUE;
  }
  while ( m_CursorX > m_FirstCharacter + m_VisibleCharsPerLine )
  {
    m_FirstCharacter++;
    bHorzChanged = TRUE;
  }
  */
  if ( bHorzChanged )
  {
    SetScrollPos( SB_HORZ, m_FirstCharacter );
  }
  if ( bScrollVert )
  {
    while ( m_CursorY < m_FirstLine )
    {
      m_FirstLine--;
      bVertChanged = TRUE;
    }
    while ( m_CursorY >= m_FirstLine + m_VisibleLines )
    {
      m_FirstLine++;
      bVertChanged = TRUE;
    }

    if ( bVertChanged )
    {
      SetScrollPos( SB_VERT, m_FirstLine );
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
  if ( (int)m_Text.size() > m_VisibleLines )
  {
    SetScrollRange( SB_VERT, 0, (int)m_Text.size() - m_VisibleLines );
  }
  else
  {
    SetScrollRange( SB_VERT, 0, 0 );
    m_FirstLine = 0;
  }

  // Werte anpassen
  GetClientRect( &m_rectRC );
  m_VisibleCharsPerLine  = ( m_rectRC.right - m_rectRC.left ) / m_CharWidth;
  m_VisibleLines         = ( m_rectRC.bottom - m_rectRC.top ) / m_CharHeight;

  // nochmal, falls sich die Randwerte geändert haben
  if ( (int)m_Text.size() > m_VisibleLines )
  {
    SetScrollRange( SB_VERT, 0, (int)m_Text.size() - m_VisibleLines );
  }
  else
  {
    SetScrollRange( SB_VERT, 0, 0 );
    m_FirstLine = 0;
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
      m_SelectionAnchorX = m_CursorX;
      m_SelectionAnchorY = m_CursorY;
    }
    else
    {
      if ( !m_Selection )
      {
        m_Selection = TRUE;
        Invalidate();
      }
    }
	}
	else
	{
    m_SelectionAnchorX = m_CursorX;
    m_SelectionAnchorY = m_CursorY;
    if ( m_Selection )
    {
      m_Selection = FALSE;
      Invalidate();
    }
	}
  if ( m_Selection )
  {
    if ( ( m_CursorX == m_SelectionAnchorX )
    &&   ( m_CursorY == m_SelectionAnchorY ) )
    {
      m_Selection = FALSE;
      Invalidate();
    }
  }

}



/*-OnChar---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{

  if ( m_SelectingByMouse )
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



/*-OnMouseMove----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnMouseMove( UINT nFlags, POINT point )
{

  if ( m_SelectingByMouse )
  {
    if ( !PtInRect( &m_rectRC, point ) )
    {
      return;
    }
    int     iX,
            iY;

    GetMouseCursorPos( point, &iX, &iY );

    m_CursorX = iX;
    m_CursorY = iY;
    if ( ( m_CursorX != m_SelectionAnchorX )
    ||   ( m_CursorY != m_SelectionAnchorY ) )
    {
      m_Selection = TRUE;
    }
    else
    {
      m_Selection = FALSE;
    }
    Invalidate();
  }

	CWnd::OnMouseMove( nFlags, point );

}



/*-OnLButtonUp----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnLButtonUp( UINT nFlags, POINT point )
{

  if ( m_SelectingByMouse )
  {
  	m_SelectingByMouse = FALSE;
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
  if ( !m_Enabled )
  {
    return;
  }


  int     iX,
          iY;

  GetMouseCursorPos( point, &iX, &iY );

  m_CursorX = iX;
  m_CursorY = iY;


  int iStartX = iX,
      iEndX = iX;

  do
  {
    if ( iStartX >= 0 )
    {
      // if ( isalnum( m_Text[iY][iStartX] ) )
      if ( IsAlphanumeric( m_Text[iY][iStartX] ) )
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
    if ( iEndX < (int)m_Text[iY].length() )
    {
      // if ( isalnum( m_Text[iY][iEndX] ) )
      if ( IsAlphanumeric( m_Text[iY][iEndX] ) )
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
    m_Selection = TRUE;
    m_CursorX = iEndX;
    m_CursorY = iY;
    m_SelectionAnchorX = iStartX + 1;
    m_SelectionAnchorY = iY;
    m_CursorWantedX = -1;
    Invalidate();
  }

	//CWnd::OnLButtonDblClk( nFlags, point );

}



void CScriptEditCtrl::GetMouseCursorPos( POINT point, int *iX, int *iY )
{

  *iY = m_FirstLine + point.y / m_CharHeight;
  if ( *iY >= (int)m_Text.size() )
  {
    *iY = (int)m_Text.size() - 1;
  }
  if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
  {
    *iX = m_FirstCharacter + ( ( point.x + m_CharWidth / 2 ) ) / m_CharWidth;
    // Cursor im Text?
    if ( *iX > (int)m_Text[*iY].length() )
    {
      *iX = (int)m_Text[*iY].length();
    }
  }
  else
  {
    if ( m_Text[*iY].length() == 0 )
    {
      *iX = 0;
      return;
    }

    int   iDummyX,
          iRealX,
          iWidth;

    iDummyX = 0;
    iRealX = 0;

    do
    {
      if ( m_Text[*iY][iDummyX] == VK_TAB )
      {
        iWidth = m_CharWidth * m_TabSpacing;
      }
      else
      {
        iWidth = m_CharWidth;
      }
      if ( ( m_FirstCharacter * m_CharWidth + point.x >= iRealX )
      &&   ( m_FirstCharacter * m_CharWidth + point.x < iRealX + iWidth ) )
      {
        // hierhin haben wir geklickt!
        if ( m_FirstCharacter * m_CharWidth + point.x - iRealX > iWidth / 2 )
        {
          // ans Ende des Buchstabens
          *iX = iDummyX + 1;
          if ( *iX > (int)m_Text[*iY].length() )
          {
            *iX = (int)m_Text[*iY].length();
          }
        }
        else
        {
          *iX = iDummyX;
        }
        return;
      }

      iDummyX++;
      if ( iDummyX >= (int)m_Text[*iY].length() )
      {
        // über den Text raus!
        *iX = (int)m_Text[*iY].length();
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

  int     iX,
          iY;


  GetMouseCursorPos( point, &iX, &iY );

  m_CursorX = iX;
  m_CursorY = iY;
  if ( !m_SelectingByMouse )
  {
    m_SelectingByMouse = TRUE;
    m_SelectionAnchorX = iX;
    m_SelectionAnchorY = iY;
    SetCapture();
  }

  ScrollIntoView();
  Invalidate();

	//CWnd::OnLButtonDown( nFlags, point );

}



/*-OnSetFocus-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnSetFocus( HWND hwndOld )
{

	//CWnd::OnSetFocus( pOldWnd );

  if ( !m_Focus )
  {
	  m_Focus = TRUE;
  }
  if ( !m_ShowCaret )
  {
    CreateSolidCaret( 1, m_CharHeight );
    ShowCaret();
    m_ShowCaret = TRUE;
  }
  DrawCursor();
  Invalidate();

}



/*-OnKillFocus----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnKillFocus( HWND hwndNew )
{

	//CWnd::OnKillFocus( pNewWnd );

  if ( m_SelectingByMouse )
  {
    m_SelectingByMouse = FALSE;
    ReleaseCapture();
  }

  if ( m_Focus )
  {
	  m_Focus = FALSE;
    Invalidate();
  }
  if ( m_ShowCaret )
  {
    m_ShowCaret = FALSE;
    HideCaret();
  }

}



/*-OnGetDlgCode---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

UINT CScriptEditCtrl::OnGetDlgCode()
{

	return DLGC_WANTALLKEYS;

}




/*-OnSetCursor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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



/*-OnVScroll------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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
      m_FirstLine = 0;
		  Invalidate();
		  break;
	  case SB_LINELEFT:
      if ( m_FirstLine > 0 )
      {
        m_FirstLine--;
  		  Invalidate();
      }
		  break;
	  case SB_PAGELEFT:
      m_FirstLine -= m_VisibleLines * 3 / 4;
      if ( m_FirstLine < 0 )
      {
        m_FirstLine = 0;
      }
		  Invalidate();
		  break;
	  case SB_LINERIGHT:
      if ( m_FirstLine < (int)m_Text.size() - m_VisibleLines )
      {
        m_FirstLine++;
  		  Invalidate();
      }
		  break;
	  case SB_PAGERIGHT:
      m_FirstLine += m_VisibleLines * 3 / 4;
      if ( m_FirstLine > (int)m_Text.size() - m_VisibleLines )
      {
        m_FirstLine = (int)m_Text.size() - m_VisibleLines;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_FirstLine = (int)m_Text.size() - m_VisibleLines;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      m_FirstLine = nPos;
		  Invalidate();
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}
	SetScrollPos( SB_VERT, m_FirstLine, TRUE );

	//CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

}



/*-OnHScroll------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CScriptEditCtrl::OnHScroll(UINT nSBCode, UINT nPos )
{

	switch( nSBCode )
	{
	  case SB_LEFT:
      m_FirstCharacter = 0;
		  Invalidate();
		  break;
	  case SB_LINELEFT:
      if ( m_FirstCharacter > 0 )
      {
        m_FirstCharacter--;
  		  Invalidate();
      }
		  break;
	  case SB_PAGELEFT:
      m_FirstCharacter -= m_VisibleCharsPerLine * 3 / 4;
      if ( m_FirstCharacter < 0 )
      {
        m_FirstCharacter = 0;
      }
		  Invalidate();
		  break;
	  case SB_LINERIGHT:
      if ( m_FirstCharacter < (int)SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_VisibleCharsPerLine )
      {
        m_FirstCharacter++;
  		  Invalidate();
      }
		  break;
	  case SB_PAGERIGHT:
      m_FirstCharacter += m_VisibleCharsPerLine * 3 / 4;
      if ( m_FirstCharacter > (int)SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_VisibleCharsPerLine )
      {
        m_FirstCharacter = SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_VisibleCharsPerLine;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_FirstCharacter = SCRIPT_EDIT_CONTROL::MAX_LINE_LENGTH - m_VisibleCharsPerLine;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      m_FirstCharacter = nPos;
		  Invalidate();
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}
	SetScrollPos( SB_HORZ, m_FirstCharacter, TRUE );

  Default();
	//CWnd::OnHScroll( nSBCode, nPos, pScrollBar );

}



/*-OnEraseBkgnd---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CScriptEditCtrl::OnEraseBkgnd( HDC hdc )
{

	return TRUE;//FALSE;

}



void CScriptEditCtrl::AddKeyWord( const char *szKeyWord, COLORREF colorDummy )
{
  CSyntaxColorEntry   sceDummy;


  sceDummy.KeyWord = szKeyWord;
  sceDummy.ColorKey = colorDummy;

  m_SyntaxEntry.push_back( sceDummy );
}



void CScriptEditCtrl::FillColorVector( const GR::String& strLineArg, int iCurrentLine, std::vector<COLORREF> *vectColors )
{
  // erst einmal alles schwarz
  if ( strLineArg.length() == 0 )
  {
    return;
  }
  for ( size_t i = 0; i < strLineArg.length(); i++ )
  {
    (*vectColors)[i] = 0;
  }
  if ( m_SyntaxEntry.size() == 0 )
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

  GR::String    strLine = strLineArg;
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
        for ( size_t i = 0; i < m_SyntaxEntry.size(); i++ )
        {
          if ( strKeyWord == m_SyntaxEntry[i].KeyWord )
          {
            // Keyword erkannt!
            for ( int j = iWordStart; j < iWordStart + iWordLength; j++ )
            {
              (*vectColors)[j] = m_SyntaxEntry[i].ColorKey;
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
  if ( m_SelectingByMouse )
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
    if ( point.y - m_rectRC.top < m_CharHeight / 2 )
    {
      // Scroll nach oben
      SendMessage( WM_VSCROLL, SB_LINELEFT, 0 );
      bMoved = TRUE;
    }
    if ( m_rectRC.bottom - point.y < m_CharHeight / 2 )
    {
      // Scroll nach unten
      SendMessage( WM_VSCROLL, SB_LINERIGHT, 0 );
      bMoved = TRUE;
    }
    if ( point.x - m_rectRC.left < m_CharWidth / 2 )
    {
      // Scroll nach links
      SendMessage( WM_HSCROLL, SB_LINELEFT, 0 );
      bMoved = TRUE;
    }
    if ( m_rectRC.right - point.x < m_CharWidth / 2 )
    {
      // Scroll nach rechts
      SendMessage( WM_HSCROLL, SB_LINERIGHT, 0 );
      bMoved = TRUE;
    }
    if ( bMoved )
    {
      int     iX,
              iY;


      iX = m_FirstCharacter + ( ( point.x + m_CharWidth / 2 ) ) / m_CharWidth;
      iY = m_FirstLine + point.y / m_CharHeight;

      if ( iY >= (int)m_Text.size() )
      {
        // unten raus, Cursor setzen
        iY = (int)m_Text.size() - 1;
        iX = (int)m_Text[iY].length();
      }
      else
      {
        // Cursor im Text?
        if ( iX > (int)m_Text[iY].length() )
        {
          iX = (int)m_Text[iY].length();
        }
      }
      m_CursorX = iX;
      m_CursorY = iY;
      Invalidate();
    }
  }

	//CWnd::OnTimer( nIDEvent );

}



void CScriptEditCtrl::SetTabSpacing( DWORD dwSpaceCount )
{
  m_TabSpacing = dwSpaceCount;
}



void CScriptEditCtrl::Enable( BOOL bEnable )
{
  if ( bEnable )
  {
    if ( !m_Enabled )
    {
      m_Enabled = TRUE;
      Invalidate();
    }
  }
  else
  {
    if ( m_Enabled )
    {
      m_Enabled = FALSE;
      Invalidate();
    }
  }

}



BOOL CScriptEditCtrl::IsEnabled()
{
  return m_Enabled;
}



void CScriptEditCtrl::SetColor( DWORD dwColor, COLORREF color )
{
  if ( dwColor >= m_vectColor.size() )
  {
    return;
  }
  m_vectColor[dwColor] = color;
}



void CScriptEditCtrl::ClipCoordinatesToText( int *iX, int *iY )
{
  if ( *iY >= (int)m_Text.size() )
  {
    *iY = (int)m_Text.size() - 1;
  }
  if ( *iY < 0 )
  {
    *iY = 0;
  }
  if ( *iX > (int)m_Text[*iY].length() )
  {
    *iX = (int)m_Text[*iY].length();
  }
  if ( *iX < 0 )
  {
    *iX = 0;
  }
}


int CScriptEditCtrl::GetCursorX()           const { return m_CursorX;          }
int CScriptEditCtrl::GetCursorY()           const { return m_CursorY;          }
int CScriptEditCtrl::GetSelectionAnchorX()  const { return m_SelectionAnchorX; }
int CScriptEditCtrl::GetSelectionAnchorY()  const { return m_SelectionAnchorY; }



void CScriptEditCtrl::SetPosition( int iX, int iY )
{

  m_CursorX = iX;
  m_CursorY = iY;
  ClipCoordinatesToText( &m_CursorX, &m_CursorY );
  ScrollIntoView();
  Invalidate();

}



void CScriptEditCtrl::SetSelection( int iX1, int iY1, int iX2, int iY2 )
{
  ClipCoordinatesToText( &iX2, &iY2 );
  ClipCoordinatesToText( &iX1, &iY1 );
  m_CursorX = iX2;
  m_CursorY = iY2;
  m_SelectionAnchorX = iX1;
  m_SelectionAnchorY = iY1;
  ScrollIntoView();
  Invalidate();
}



void CScriptEditCtrl::OnClose()
{

	KillTimer( 1 );

	//CWnd::OnClose();

}



BOOL CScriptEditCtrl::FindString( GR::String strFindMe, int *iX, int *iY )
{
  for ( size_t i = 0; i < m_Text.size(); i++ )
  {
    size_t iPos = (size_t)m_Text[i].find( strFindMe, 0 );
    if ( iPos != GR::String::npos )
    {
      // gefunden!
      *iX = (int)iPos;
      *iY = (int)i;
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
  if ( !m_Selection )          return;
  if ( m_Text.size() <= 0 ) return;

  int             iX1,
                  iY1,
                  iX2,
                  iY2;
  GR::String     strDummy;

  if ( m_Selection )
  {
    iX1 = m_SelectionAnchorX;
    iY1 = m_SelectionAnchorY;
    iX2 = m_CursorX;
    iY2 = m_CursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_CursorX;
      iY1 = m_CursorY;
      iX2 = m_SelectionAnchorX;
      iY2 = m_SelectionAnchorY;
    }
  }
  else
  {
    iX1 = 0;
    iY1 = 0;
    iX2 = (int)m_Text[m_Text.size() - 1].length();
    iY2 = (int)m_Text.size() - 1;
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
    int   iChar = iX1,
          iZeile = iY1;

    strDummy = "";
    do
    {
      if ( iZeile < (int)m_Text.size() )
      {
        if ( iChar < (int)m_Text[iZeile].length() )
        {
          strDummy += m_Text[iZeile][iChar];
        }
        iChar++;
        if ( ( iZeile < iY2 )
        ||   ( ( iZeile == iY2 )
        &&     ( iChar < iX2 ) ) )
        {
          if ( iChar >= (int)m_Text[iZeile].length() )
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
		strcpy_s( buffer, (unsigned int)( strDummy.length() + 1 ), strDummy.c_str() );
		GlobalUnlock( clipbuffer );
		SetClipboardData( CF_TEXT, clipbuffer );
		CloseClipboard();

    if ( m_Selection )
    {
      RemoveSelection();
    }
    else
    {
      Reset();
    }
    m_Modified = TRUE;
    AdjustScrollBars();
    Invalidate();
  }
}


GR::String CScriptEditCtrl::GetSelection()
{

  if ( m_Text.empty() ) return GR::String();

  int             iX1, iY1, iX2, iY2;
  GR::String     strDummy;

  if ( m_Selection )
  {
    iX1 = m_SelectionAnchorX;
    iY1 = m_SelectionAnchorY;
    iX2 = m_CursorX;
    iY2 = m_CursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_CursorX;
      iY1 = m_CursorY;
      iX2 = m_SelectionAnchorX;
      iY2 = m_SelectionAnchorY;
    }
  }
  else
  {
    return GR::String();
    // iX1 = 0;
    // iY1 = 0;
    // iX2 = m_Text[m_Text.size() - 1].length();
    // iY2 = m_Text.size() - 1;
  }

  if ( ( iX1 == iX2 )  &&   ( iY1 == iY2 ) ) return GR::String();

  int   iChar = iX1, iZeile = iY1;
  strDummy = "";
  do
  {
    if ( iZeile < (int)m_Text.size() )
    {
      if ( iChar < (int)m_Text[iZeile].length() )
      {
        strDummy += m_Text[iZeile][iChar];
      }
      iChar++;
      if ( ( iZeile < iY2 )
      ||   ( ( iZeile == iY2 )
      &&     ( iChar < iX2 ) ) )
      {
        if ( iChar >= (int)m_Text[iZeile].length() )
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
  if ( !m_Selection )          return;
  if ( m_Text.size() <= 0 ) return;

  int             iX1,
                  iY1,
                  iX2,
                  iY2;
  GR::String     strDummy;

  if ( m_Selection )
  {
    iX1 = m_SelectionAnchorX;
    iY1 = m_SelectionAnchorY;
    iX2 = m_CursorX;
    iY2 = m_CursorY;
    if ( !IsAbove( iX1, iY1, iX2, iY2 ) )
    {
      // Koordinaten tauschen
      iX1 = m_CursorX;
      iY1 = m_CursorY;
      iX2 = m_SelectionAnchorX;
      iY2 = m_SelectionAnchorY;
    }
  }
  else
  {
    iX1 = 0;
    iY1 = 0;
    iX2 = (int)m_Text[m_Text.size() - 1].length();
    iY2 = (int)m_Text.size() - 1;
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
    int   iChar = iX1,
          iZeile = iY1;

    strDummy = "";
    do
    {
      if ( iZeile < (int)m_Text.size() )
      {
        if ( iChar < (int)m_Text[iZeile].length() )
        {
          strDummy += m_Text[iZeile][iChar];
        }
        iChar++;
        if ( ( iZeile < iY2 )
        ||   ( ( iZeile == iY2 )
        &&     ( iChar < iX2 ) ) )
        {
          if ( iChar >= (int)m_Text[iZeile].length() )
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
		strcpy_s( buffer, strDummy.length() + 1, strDummy.c_str() );
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
          int                                   iDummy = m_CursorY;


          itVectString = m_Text.begin();
          while ( iDummy )
          {
            itVectString++;
            iDummy--;
          }
          m_Text.insert( itVectString, "" );
          if ( m_CursorX == m_Text[m_CursorY + 1].length() )
          {
            m_Text[m_CursorY] = m_Text[m_CursorY + 1] + strDummy;
            m_CursorY++;
            m_Text[m_CursorY] = "";
          }
          else
          {
            m_Text[m_CursorY] = m_Text[m_CursorY + 1].substr( 0, m_CursorX ) + strDummy;
            m_CursorY++;
            if ( m_Text[m_CursorY].length() > 0 )
            {
              m_Text[m_CursorY] = m_Text[m_CursorY].substr( m_CursorX );
            }
            else
            {
              m_Text[m_CursorY] = "";
            }
            m_CursorX = 0;
          }
          strDummy = "";
        }
        else if ( cDummy != 13 )
        {
          if ( m_Flags & SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS )
          {
            if ( cDummy == VK_TAB )
            {
              for ( DWORD i = 0; i < m_TabSpacing; i++ )
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
    m_Modified = TRUE;
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



void CScriptEditCtrl::AppendText( const GR::String& strText )
{
  // nach ganz unten
  SetSelection( m_CursorX, m_CursorY, m_CursorX, m_CursorY );
  int   iOldX = m_CursorX;
  int   iOldY = m_CursorY;

  m_CursorX = (int)m_Text[m_Text.size() - 1].length();
  m_CursorY = (int)m_Text.size() - 1;

  InsertString( strText );

  if ( ::GetFocus() == m_hWnd )
  {
    m_CursorX = iOldX;
    m_CursorY = iOldY;
  }
  ScrollIntoView();
  Invalidate();

}



