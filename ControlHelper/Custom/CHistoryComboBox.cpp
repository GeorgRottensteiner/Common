/*--------------------+-------------------------------------------------------+
 | Programmname       : ListControl-Helper                                    |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 17.01.2002                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <windows.h>

#include "CHistoryComboBox.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CHistoryComboBox::CHistoryComboBox() :
  CComboBox(),
  m_bBackSpace( false ),
  m_bDelete( false )
{
}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CHistoryComboBox::~CHistoryComboBox()
{
}



/*-AddToHistory---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CHistoryComboBox::AddToHistory( const GR::String& strText )
{

}



/*-AddString------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

LRESULT CHistoryComboBox::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_KEYDOWN:
      break;
  }


  return CComboBox::WindowProc( uMsg, wParam, lParam );

}



BOOL CHistoryComboBox::PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
{

  if ( uMsg == WM_KEYDOWN )
  {
    MSG   msg;

    msg.hwnd    = hWnd;
    msg.message = uMsg;
    msg.wParam  = wParam;
    msg.lParam  = lParam;

    m_bBackSpace  = ( wParam == VK_BACK );
    m_bDelete     = ( wParam == VK_DELETE );

    if ( ( !m_bBackSpace )
    &&   ( !m_bDelete )
    &&   ( wParam < 0x30 ) )
    {
      return FALSE;
    }
    TranslateMessage( &msg );
    lResult = DispatchMessage( &msg );
    return TRUE;
  }
  else if ( uMsg == WM_CHAR )
  {
    if ( ( !m_bBackSpace )
    &&   ( !m_bDelete )
    &&   ( wParam < 32 ) )
    {
      return FALSE;
    }

    MSG   msg;

    msg.hwnd    = hWnd;
    msg.message = uMsg;
    msg.wParam  = wParam;
    msg.lParam  = lParam;

    TranslateMessage( &msg );
    lResult = DispatchMessage( &msg );

    DWORD     dwSelPos,
              dwSelEnd;

    ::SendMessage( hWnd, EM_GETSEL, (WPARAM)&dwSelPos, (LPARAM)&dwSelEnd );

    GR::String     strFindText = GetWindowText();

    if ( ( m_bBackSpace )
    ||   ( m_bDelete ) )
    {
      return TRUE;
    }



    strFindText = strFindText.substr( 0, dwSelPos );

    int   iItem = FindString( 0, strFindText.c_str() );

    if ( iItem != CB_ERR )
    {
      SetCurSel( iItem );

      GR::String     strFindText = GetWindowText();

      SetEditSel( dwSelPos, -1 );
    }
    return TRUE;
  }


  return FALSE;

}