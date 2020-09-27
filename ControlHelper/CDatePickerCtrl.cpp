#include <windows.h>

#pragma warning( disable:4786 )
#include <map>

#include <Debug\debugclient.h>

#include <ControlHelper\CButton.h>
#include <ControlHelper\CDialog.h>
#include <ControlHelper\CDatePickerCtrl.h>



namespace WindowsWrapper
{

  CDatePickerCtrl::CDatePickerCtrl() :
    CDialog()
  {

    _RegisterClass();

  }



  CDatePickerCtrl::~CDatePickerCtrl()
  {


  }



  BOOL CDatePickerCtrl::Create( HWND hwndParent,
                                DWORD dwStyle,
                                DWORD dwID )
  {
    bool    bVisible = false;

    if ( dwStyle & WS_VISIBLE )
    {
      bVisible = true;
    }

    dwStyle &= ~WS_VISIBLE;
    dwStyle |= WS_DLGFRAME;

    RECT    rc;

    rc.left = 0;
    rc.top = 0;
    rc.right = rc.left + 320;
    rc.bottom = rc.top + 240;

    BOOL    bResult = CWnd::Create( "DATEPICKERCTRLCLASS",
                                    "DatePicker",
                                    dwStyle,
                                    rc,
                                    hwndParent,
                                    dwID );

    if ( bResult )
    {
      m_hwndParent = hwndParent;
    }

    CenterWindow();

    GetClientRect( &rc );

    // OK-Button
    rc.left = 110;
    rc.top = 200;
    rc.right = rc.left + 100;
    rc.bottom = rc.top + 20;

    m_buttonOK.Create( "OK", WS_VISIBLE | WS_CHILD, rc, m_hWnd, 11 );
    m_buttonOK.SetFont( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );
    m_hwndButtonOK = m_buttonOK.m_hWnd;

    // Monat hoch
    rc.left = 230;
    rc.top = 35;
    rc.right = rc.left + 30;
    rc.bottom = rc.top + 18;

    m_buttonMonthDown.Create( ">", WS_VISIBLE | WS_CHILD, rc, m_hWnd, 12 );
    m_buttonMonthDown.SetFont( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );
    m_hwndButtonMonthDown = m_buttonMonthDown.m_hWnd;

    // Monat runter
    rc.left = 60;
    rc.top = 35;
    rc.right = rc.left + 30;
    rc.bottom = rc.top + 18;

    m_buttonMonthUp.Create( "<", WS_VISIBLE | WS_CHILD, rc, m_hWnd, 13 );
    m_buttonMonthUp.SetFont( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );
    m_hwndButtonMonthUp = m_buttonMonthUp.m_hWnd;

    m_ShowDate = GR::DateTime::DateTime::Now();

    //SYSTEMTIME      stDummy = m_ShowDate;

    SYSTEMTIME      stDummy;

    GetLocalTime( &stDummy );

    m_iCurrentMonth = stDummy.wMonth;
    m_iCurrentYear = stDummy.wYear;

    m_iSelectedDay = stDummy.wDay;
    m_iSelectedMonth = m_iCurrentMonth;
    m_iSelectedYear = m_iCurrentYear;

    m_iActDay = stDummy.wDay;
    m_iActMonth = m_iCurrentMonth;
    m_iActYear = m_iCurrentYear;

    while ( stDummy.wDayOfWeek )
    {
      FILETIME    ft;

      SystemTimeToFileTime( &stDummy, &ft );

      hyper   llDummy;

      memcpy( &llDummy, &ft, sizeof( LARGE_INTEGER ) );

      llDummy -= (hyper)10 * (hyper)1000 * (hyper)1000 * (hyper)60 * (hyper)60 * (hyper)24; // ein Tag abziehen

      memcpy( &ft, &llDummy, sizeof( LARGE_INTEGER ) );

      FileTimeToSystemTime( &ft, &stDummy );
    }

    char    szTemp[200];

    for ( int i = 0; i < 7; i++ )
    {
      GetDateFormatA( LOCALE_USER_DEFAULT,
                      0,             // flags specifying function options
                      &stDummy,
                      "ddd",
                      szTemp,
                      198 );
      m_strDayOfWeek[( i + 6 ) % 7] = szTemp;



      FILETIME    ft;

      SystemTimeToFileTime( &stDummy, &ft );


      hyper   llDummy;

      memcpy( &llDummy, &ft, sizeof( LARGE_INTEGER ) );

      llDummy += (hyper)10 * (hyper)1000 * (hyper)1000 * (hyper)60 * (hyper)60 * (hyper)24; // ein Tag abziehen

      memcpy( &ft, &llDummy, sizeof( LARGE_INTEGER ) );

      FileTimeToSystemTime( &ft, &stDummy );
    }

    // die Monatsnamen
    GetLocalTime( &stDummy );
    for ( int i = 0; i < 12; i++ )
    {
      stDummy.wMonth = i + 1;
      stDummy.wDay = 1;
      GetDateFormatA( LOCALE_USER_DEFAULT,
                      0,             // flags specifying function options
                      &stDummy,
                      "MMMM",
                      szTemp,
                      198 );
      m_strMonth[i] = szTemp;
    }

    if ( bVisible )
    {
      ShowWindow( SW_SHOW );
    }

    Invalidate();

    return bResult;
  }



  void CDatePickerCtrl::OnPaint( HDC hdc )
  {
    SYSTEMTIME    sysTime,
      stCurrent,
      stFirstDayOfMonth;

    RECT          rc;


    GetLocalTime( &sysTime );
    GetClientRect( &rc );

    HDC   newHDC = ::GetDC( m_hWnd );
    FillRect( newHDC, &rc, (HBRUSH)( COLOR_BTNFACE + 1 ) );

    SelectObject( newHDC, GetStockObject( DEFAULT_GUI_FONT ) );
    SetBkMode( newHDC, TRANSPARENT );

    char    szTemp[200];

    rc.left = 20;
    rc.top = 10;
    rc.right = 300;
    rc.bottom = rc.top + 20;

    wsprintfA( szTemp, "%d. %s %d", m_iSelectedDay, m_strMonth[m_iSelectedMonth - 1].c_str(), m_iSelectedYear );

    GR::WString   utf16String = GR::Convert::ToUTF16( szTemp );

    SetTextColor( newHDC, GetSysColor( COLOR_WINDOWTEXT ) );
    ::DrawTextW( newHDC, utf16String.c_str(), (int)utf16String.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );


    rc.left = 20;
    rc.top = 35;
    rc.right = 300;
    rc.bottom = rc.top + 20;

    wsprintfA( szTemp, "%s %d", m_strMonth[m_iCurrentMonth - 1].c_str(), m_iCurrentYear );
    utf16String = GR::Convert::ToUTF16( szTemp );

    ::DrawTextW( newHDC, utf16String.c_str(), (int)utf16String.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

    for ( int i = 0; i < 7; i++ )
    {
      rc.left = 20 + i * 40;
      rc.top = 60;
      rc.right = rc.left + 40;
      rc.bottom = rc.top + 20;

      //DrawFrameControl( newHDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH );
      DrawEdge( newHDC, &rc, EDGE_ETCHED, BF_RECT );

      if ( i == 6 )
      {
        SetTextColor( newHDC, RGB( 255, 0, 0 ) );
      }
      ::DrawTextW( newHDC, GR::Convert::ToUTF16( m_strDayOfWeek[i] ).c_str(), (int)GR::Convert::ToUTF16( m_strDayOfWeek[i] ).length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
    }


    stCurrent = sysTime;

    sysTime.wYear = m_iCurrentYear;
    sysTime.wMonth = m_iCurrentMonth;
    sysTime.wDay = 1;

    int iCurrentDay = sysTime.wDay;

    FILETIME    ftCurrent,
      ftFirstDayOfMonth;

    ULONGLONG       li;

    // den ersten des Monats rausrechnen
    SystemTimeToFileTime( &sysTime, &ftCurrent );

    FileTimeToSystemTime( &ftCurrent, &sysTime );

    li = ( ( (ULONGLONG)ftCurrent.dwHighDateTime ) << 32 ) + ftCurrent.dwLowDateTime;
    memcpy( &stFirstDayOfMonth, &sysTime, sizeof( SYSTEMTIME ) );
    ftFirstDayOfMonth = ftCurrent;
    iCurrentDay = 0;
    int iPos = ( sysTime.wDayOfWeek + 6 ) % 7;

    SetTextColor( newHDC, GetSysColor( COLOR_WINDOWTEXT ) );

    do
    {
      if ( iCurrentDay >= 32 )
      {
        // ohje!
        break;
      }
      // einen Tag draufzählen
      li += (ULONGLONG)864000000000;   // ms pro Tag
      ftCurrent.dwLowDateTime = (DWORD)( li & 0xFFFFFFFF );
      ftCurrent.dwHighDateTime = (DWORD)( li >> 32 );
      FileTimeToSystemTime( &ftCurrent, &sysTime );

      RECT    rc;

      rc.left = 20 + ( iPos % 7 ) * 40;
      rc.top = 90 + ( iPos / 7 ) * 20;
      rc.right = rc.left + 40;
      rc.bottom = rc.top + 20;

      if ( ( m_iSelectedDay == iCurrentDay + 1 )
           && ( m_iCurrentMonth == m_iSelectedMonth )
           && ( m_iCurrentYear == m_iSelectedYear ) )
      {
        DrawFrameControl( newHDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED );
      }
      else
      {
        DrawFrameControl( newHDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED | DFCS_FLAT );
      }

      wsprintfA( szTemp, "%d", iCurrentDay + 1 );
      utf16String = GR::Convert::ToUTF16( szTemp );

      // selektierter Tag
      if ( ( m_iSelectedDay == iCurrentDay + 1 )
           && ( m_iCurrentMonth == m_iSelectedMonth )
           && ( m_iCurrentYear == m_iSelectedYear ) )
      {
        InflateRect( &rc, -2, -2 );

        FillRect( newHDC, &rc, (HBRUSH)GetStockObject( WHITE_BRUSH ) );

        InflateRect( &rc, 2, 2 );

        ::DrawTextW( newHDC, utf16String.c_str(), (int)utf16String.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
      }
      else if ( ( iCurrentDay + 1 == m_iActDay )
                && ( m_iCurrentMonth == m_iActMonth )
                && ( m_iCurrentYear == m_iActYear ) )
      {
        // heutiger Tag
        InflateRect( &rc, -2, -2 );

        FillRect( newHDC, &rc, (HBRUSH)( COLOR_HIGHLIGHT + 1 ) );

        InflateRect( &rc, 2, 2 );

        SetTextColor( newHDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        ::DrawTextW( newHDC, utf16String.c_str(), (int)utf16String.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
        SetTextColor( newHDC, GetSysColor( COLOR_WINDOWTEXT ) );
      }
      else
      {
        ::DrawTextW( newHDC, utf16String.c_str(), (int)utf16String.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
      }

      iPos++;
      iCurrentDay++;
    }
    while ( sysTime.wMonth == stFirstDayOfMonth.wMonth );


    ::ReleaseDC( m_hWnd, newHDC );


    ::ValidateRect( m_hWnd, NULL );

    ::InvalidateRect( m_hwndButtonOK, NULL, FALSE );
    ::InvalidateRect( m_hwndButtonMonthUp, NULL, FALSE );
    ::InvalidateRect( m_hwndButtonMonthDown, NULL, FALSE );
    Default();

  }



  void CDatePickerCtrl::_RegisterClass()
  {
    WNDCLASSW    wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ::DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE)GetModuleHandle( NULL );
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"DATEPICKERCTRLCLASS";
    RegisterClassW( &wc );
  }



  void CDatePickerCtrl::OnButtonDown( POINT& ptPos )
  {

    SYSTEMTIME    sysTime,
      stCurrent,
      stFirstDayOfMonth;


    GetLocalTime( &sysTime );

    /*
    sysTime.wYear   = m_ShowDate.wYear;
    sysTime.wMonth  = m_ShowDate.wMonthwMonth;
    */
    sysTime.wDay = 1;
    sysTime.wMonth = m_iCurrentMonth;
    sysTime.wYear = m_iCurrentYear;

    stCurrent = sysTime;

    int iCurrentDay = sysTime.wDay;

    FILETIME    ftCurrent,
      ftFirstDayOfMonth;

    ULONGLONG       li;

    // den ersten des Monats rausrechnen
    SystemTimeToFileTime( &sysTime, &ftCurrent );

    FileTimeToSystemTime( &ftCurrent, &sysTime );

    li = ( ( (ULONGLONG)ftCurrent.dwHighDateTime ) << 32 ) + ftCurrent.dwLowDateTime;
    memcpy( &stFirstDayOfMonth, &sysTime, sizeof( SYSTEMTIME ) );
    ftFirstDayOfMonth = ftCurrent;
    iCurrentDay = 0;
    int iPos = ( sysTime.wDayOfWeek + 6 ) % 7;

    do
    {
      if ( iCurrentDay >= 32 )
      {
        // ohje!
        break;
      }
      // einen Tag draufzählen
      li += (ULONGLONG)864000000000;   // ms pro Tag
      ftCurrent.dwLowDateTime = (DWORD)( li & 0xFFFFFFFF );
      ftCurrent.dwHighDateTime = (DWORD)( li >> 32 );
      FileTimeToSystemTime( &ftCurrent, &sysTime );

      RECT    rc;

      rc.left = 20 + ( iPos % 7 ) * 40;
      rc.top = 90 + ( iPos / 7 ) * 20;
      rc.right = rc.left + 40;
      rc.bottom = rc.top + 20;

      if ( PtInRect( &rc, ptPos ) )
      {
        m_iSelectedDay = iCurrentDay + 1;
        m_iSelectedMonth = m_iCurrentMonth;
        m_iSelectedYear = m_iCurrentYear;

        InvalidateRect( &rc );
        return;
      }

      iPos++;
      iCurrentDay++;
    }
    while ( sysTime.wMonth == stFirstDayOfMonth.wMonth );

  }



  /*-PreTranslateMessage--------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/

  BOOL CDatePickerCtrl::PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {

    if ( hwnd == m_hWnd )
    {
      if ( uMsg == WM_NCHITTEST )
      {
        int   iX = LOWORD( lParam ),
          iY = HIWORD( lParam );

        lResult = HTCLIENT;
        return TRUE;
      }
      /*
      if ( uMsg == WM_ERASEBKGND )
      {
        return TRUE;
      }
      */
      if ( uMsg == WM_KEYDOWN )
      {
        if ( wParam == VK_ESCAPE )
        {
          EndDialog( IDCANCEL );
        }
      }
      if ( uMsg == WM_NCPAINT )
      {
        Default();
        return TRUE;
      }
      if ( uMsg == WM_PAINT )
      {
        OnPaint( (HDC)wParam );
        return TRUE;
      }
      if ( uMsg == WM_LBUTTONDOWN )
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        SetFocus();
        OnButtonDown( pt );
      }
      if ( uMsg == WM_CREATE )
      {
        CenterWindow();
      }
    }

    return CDialog::PreTranslateMessage( hwnd, uMsg, wParam, lParam, lResult );

  }



  /*-WindowProc-----------------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/

  LRESULT CDatePickerCtrl::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    switch ( uMsg )
    {
      case WM_COMMAND:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
          if ( LOWORD( wParam ) == 11 )
          {
            // der OK-Button
            EndDialog( IDOK );
          }
          else if ( LOWORD( wParam ) == 12 )
          {
            // der Monat-Up-Button
            m_iCurrentMonth++;
            if ( m_iCurrentMonth >= 13 )
            {
              m_iCurrentMonth = 1;
              m_iCurrentYear++;
            }
            Invalidate();
          }
          else if ( LOWORD( wParam ) == 13 )
          {
            // der Monat-Down-Button
            m_iCurrentMonth--;
            if ( m_iCurrentMonth <= 0 )
            {
              m_iCurrentMonth = 12;
              m_iCurrentYear--;
            }
            Invalidate();
          }
        }
        break;
    }

    return CDialog::WindowProc( uMsg, wParam, lParam );

  }



  SYSTEMTIME CDatePickerCtrl::PickedDate() const
  {
    SYSTEMTIME    sysTime;

    sysTime.wDay = m_iSelectedDay;
    sysTime.wMonth = m_iSelectedMonth;
    sysTime.wYear = m_iSelectedYear;

    sysTime.wDayOfWeek = 0;
    sysTime.wHour = 0;
    sysTime.wMilliseconds = 0;
    sysTime.wMinute = 0;
    sysTime.wSecond = 0;

    return sysTime;
  }

}
