// e:\projekte\common\mfc\PromptEdit.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "PromptEdit.h"


// CPromptEdit

IMPLEMENT_DYNAMIC(CPromptEdit, CEdit)
CPromptEdit::CPromptEdit() :
  m_strPromptText( "" ),
  m_hFontNormal( NULL ),
  m_hFontItalic( NULL ),
  m_bActive( false ),
  m_bShowingPromptText( true )
{
}

CPromptEdit::~CPromptEdit()
{
}


BEGIN_MESSAGE_MAP(CPromptEdit, CEdit)
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_DESTROY()
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CPromptEdit-Meldungshandler

void CPromptEdit::SetPromptText( const char* szPrompt )
{

  Initialize();

  m_strPromptText = szPrompt;
  if ( !m_bActive )
  {
    if ( m_strEditText.GetLength() == 0 )
    {
      CEdit::SetWindowText( m_strPromptText );
    }
  }

}



void CPromptEdit::Initialize()
{

  if ( m_hFontNormal == NULL )
  {
    LOGFONT   lFont;

    m_hFontNormal   = (HFONT)::SendMessage( m_hWnd, WM_GETFONT, 0, 0 );
    if ( m_hFontNormal == NULL )
    {
      m_hFontNormal = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
    }
    ::GetObject( m_hFontNormal, sizeof( LOGFONT ), &lFont);

    lFont.lfWeight = 0;
    lFont.lfItalic = 0;
    m_hFontItalic   = CreateFontIndirect( &lFont );

    if ( !m_bActive )
    {
      SendMessage( WM_SETFONT, (WPARAM)m_hFontItalic, 1 );
    }
  }

}



void CPromptEdit::OnKillFocus(CWnd* pNewWnd)
{

  ToItalic();

  CEdit::OnKillFocus(pNewWnd);

}

void CPromptEdit::OnSetFocus(CWnd* pOldWnd)
{
  
  ToNormal();

  CEdit::OnSetFocus(pOldWnd);

}



void CPromptEdit::ToItalic()
{

  Initialize();

  m_bActive = false;
  CEdit::GetWindowText( m_strEditText );
  if ( m_strEditText.GetLength() == 0 )
  {
    m_bShowingPromptText = true;
    SendMessage( WM_SETFONT, (WPARAM)m_hFontItalic, 0 );
    CEdit::SetWindowText( m_strPromptText );
  }
  else
  {
    m_bShowingPromptText = false;
  }

}



void CPromptEdit::ToNormal()
{

  Initialize();

  SendMessage( WM_SETFONT, (WPARAM)m_hFontNormal, 0 );
  CEdit::SetWindowText( m_strEditText );

  m_bActive = true;
  m_bShowingPromptText = false;

}



void CPromptEdit::OnDestroy()
{
  CEdit::OnDestroy();

  DeleteObject( m_hFontItalic );

}



BOOL CPromptEdit::PreTranslateMessage(MSG* pMsg)
{

  if ( pMsg->hwnd == GetSafeHwnd() )
  {
    if ( pMsg->message == WM_PAINT )
    {
      Initialize();
    }
  }

  return CEdit::PreTranslateMessage(pMsg);
}



void CPromptEdit::GetWindowText( CString& rString ) const
{

  if ( m_bActive )
  {
    return CWnd::GetWindowText( rString );
  }

  rString = m_strEditText;

}



int CPromptEdit::GetWindowTextLength() const
{

  if ( m_bActive )
  {
    return CWnd::GetWindowTextLength();
  }

  return m_strEditText.GetLength();

}



bool CPromptEdit::IsActive()
{

  return m_bActive;

}



HBRUSH CPromptEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{

  pDC->SetBkMode( TRANSPARENT );
  if ( ( !m_bActive )
  &&   ( m_bShowingPromptText ) )
  {
    pDC->SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
  }
  else
  {
    pDC->SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
  }

  if ( IsWindowEnabled() )
  {
    return (HBRUSH)GetSysColorBrush( COLOR_WINDOW );
  }
  
  return (HBRUSH)GetSysColorBrush( COLOR_3DFACE );

}



void CPromptEdit::SetWindowText( const CString& rString )
{

  if ( m_bActive )
  {
    SendMessage( WM_SETFONT, (WPARAM)m_hFontNormal, 0 );
    CEdit::SetWindowText( rString );
  }
  else
  {
    m_strEditText = rString;
    if ( m_strEditText.GetLength() == 0 )
    {
      m_bShowingPromptText = true;
      SendMessage( WM_SETFONT, (WPARAM)m_hFontItalic, 0 );
      CEdit::SetWindowText( m_strPromptText );
    }
    else
    {
      m_bShowingPromptText = false;
      SendMessage( WM_SETFONT, (WPARAM)m_hFontNormal, 0 );
      CEdit::SetWindowText( m_strEditText );
    }
  }

}



void CPromptEdit::SetFont( CFont* pFont, BOOL bRedraw )
{

  CEdit::SetFont( pFont, bRedraw );

  Initialize();

}