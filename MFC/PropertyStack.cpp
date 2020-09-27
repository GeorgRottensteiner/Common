#include "stdafx.h"
#include "PropertyStack.h"

#include <MFC/FileDialogEx.h>

#include <Misc/Misc.h>

#include <debug/debugclient.h>
#include ".\propertystack.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



BOOL CPropertyStack::hasclass = CPropertyStack::RegisterMe();

const char CPropertyStack::szNotifyMessage[] = "WM_GRPS_NOTIFY";

GR::u32 CPropertyStack::m_dwNotifyMessage = 0;



BOOL CPropertyStack::RegisterMe()
{
  WNDCLASS wc;
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu
  wc.lpszClassName = PROPERTYSTACK_CLASS_NAME;

  m_dwNotifyMessage = RegisterWindowMessage( szNotifyMessage );
  return AfxRegisterClass(&wc);
}



CPropertyStack::CPropertyStack()
: m_iItemWidth( -1 ),
  m_iItemHeight( 14 ),
  m_iOffset( 0 ),
  m_iVisibleItems( 0 ),
  m_iSelectedItem( -1 ),
  m_iFocusedItem( -1 ),
  m_bHasFocus( false ),
  m_iTimerID( -1 ),
  m_bEditingItem( false ),
  m_bEditingLabel( false ),
  m_iEditItem( -1 )
{
}

CPropertyStack::~CPropertyStack()
{
}


BEGIN_MESSAGE_MAP(CPropertyStack, CWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
  ON_WM_LBUTTONUP()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_CHAR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPropertyStack

/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CPropertyStack::Create( RECT &rc, CWnd *wndParent, int iID )
{

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, NULL, NULL,
                        WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, wndParent, iID ) )
  {
    return FALSE;
  }

  return TRUE;

}



void CPropertyStack::Initialize( int iItemHeight )
{

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

  m_iItemHeight = iItemHeight;
  if ( m_iItemHeight < 16 )
  {
    m_iItemHeight = 16;
  }

  RECT    rc;

  GetClientRect( &rc );
  m_iVisibleItems = rc.right / m_iItemHeight;

}



void CPropertyStack::OnDestroy()
{

	CWnd::OnDestroy();

  m_vectItems.clear();

}



void CPropertyStack::SetItemHeight( int iHeight )
{

  if ( iHeight > 0 )
  {
    m_iItemHeight = iHeight;
  }

  RECT    rc;

  GetClientRect( &rc );
  m_iVisibleItems = rc.right / m_iItemHeight;

  AdjustScrollBars();
  Invalidate();

}



size_t CPropertyStack::AddString( const GR::String& strDesc, const GR::String& strItem, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_TEXT ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( strItem, dwItemData ) );
  Item.m_dwItemData       = dwItemData;

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddStatic( const GR::String& strDesc, const GR::String& strItem, bool bBold, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_STATIC_TEXT ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( strItem, dwItemData ) );
  Item.m_dwItemData       = dwItemData;
  Item.m_iValue           = bBold ? 1 : 0;

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddNumber( const GR::String& strDesc, int iValue, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_NUMBER ) );

  tPropertyItem&    Item = m_vectItems.back();

  char    szDummy[MAX_PATH];
  wsprintf( szDummy, "%d", iValue );

  Item.m_strDescription   = strDesc;
  Item.m_iValue           = iValue;
  Item.m_dwItemData       = dwItemData;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( szDummy, dwItemData ) );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddCheckBox( const GR::String& strDesc, bool bChecked, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_CHECKBOX ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_iValue           = bChecked;
  Item.m_dwItemData       = dwItemData;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( strDesc, dwItemData ) );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddColor( const GR::String& strDesc, GR::u32 dwColor, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_COLOR ) );

  tPropertyItem&    Item = m_vectItems.back();

  char    szDummy[MAX_PATH];
  wsprintf( szDummy, "#%02X%02X%02X",
           ( dwColor & 0xff0000 ) >> 16,
           ( dwColor & 0x00ff00 ) >>  8,
           ( dwColor & 0x0000ff )       );

  Item.m_strDescription   = strDesc;
  Item.m_iValue           = dwColor;
  Item.m_dwItemData       = dwItemData;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( szDummy, dwItemData ) );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddFile( const GR::String& strDesc, const GR::String& strItem, const GR::String& Filter, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_FILE ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_dwItemData       = dwItemData;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( strItem, dwItemData ) );
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( Filter, dwItemData ) );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddCustom( const GR::String& strDesc, const GR::String& strItem, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_CUSTOM ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_dwItemData       = dwItemData;
  Item.m_vectItems.push_back( tPropertyItem::tComboPair( strItem, dwItemData ) );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddCombo( const GR::String& strDesc, GR::up dwItemData )
{

  m_vectItems.push_back( tPropertyItem( E_COMBO ) );

  tPropertyItem&    Item = m_vectItems.back();

  Item.m_strDescription   = strDesc;
  Item.m_dwItemData       = dwItemData;
  Item.m_iValue           = CB_ERR;

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



size_t CPropertyStack::AddComboItem( size_t iComboIndex, const GR::String& strDesc, GR::up dwItemData )
{

  if ( iComboIndex >= m_vectItems.size() )
  {
    return CB_ERR;
  }

  if ( m_vectItems[iComboIndex].m_Type != E_COMBO )
  {
    return CB_ERR;
  }

  m_vectItems[iComboIndex].m_vectItems.push_back( tPropertyItem::tComboPair( strDesc, dwItemData ) );

  return m_vectItems[iComboIndex].m_vectItems.size() - 1;

}



size_t CPropertyStack::DeleteString( size_t nIndex )
{

  if ( nIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  std::vector<tPropertyItem>::iterator   it( m_vectItems.begin() );

  std::advance( it, nIndex );

  m_vectItems.erase( it );

  if ( m_iSelectedItem == nIndex )
  {
    m_iSelectedItem = -1;
  }
  if ( m_iFocusedItem == nIndex )
  {
    m_iFocusedItem = -1;
  }
  Invalidate();
  return m_vectItems.size();

}



GR::String CPropertyStack::GetItemDesc( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return "";
  }

  return m_vectItems[iIndex].m_strDescription;

}



GR::String CPropertyStack::GetItemText( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return "";
  }

  return m_vectItems[iIndex].m_vectItems.front().first;

}



int CPropertyStack::GetItemValue( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return 0;
  }

  return m_vectItems[iIndex].m_iValue;

}



void CPropertyStack::SetComboSelection( size_t iIndex, size_t iComboItemIndex )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return;
  }
  if ( iComboItemIndex >= m_vectItems[iIndex].m_vectItems.size() )
  {
    m_vectItems[iIndex].m_iValue = -1;
    return;
  }
  m_vectItems[iIndex].m_iValue = (int)iComboItemIndex;
  InvalidateItem( iIndex );

}



size_t CPropertyStack::GetComboSel( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return 0;
  }

  return m_vectItems[iIndex].m_iValue;

}



GR::up CPropertyStack::GetComboItemData( size_t iIndex, size_t iComboItemIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return 0;
  }

  if ( iComboItemIndex >= m_vectItems[iIndex].m_vectItems.size() )
  {
    return 0;
  }
  return m_vectItems[iIndex].m_vectItems[iComboItemIndex].second;

}



int CPropertyStack::SetItemText( size_t Index, const GR::String& Text )
{

  if ( Index >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  m_vectItems[Index].m_vectItems[0].first = Text;
  Invalidate();

  return 0;

}



int CPropertyStack::SetItemData( size_t iIndex, GR::up dwData )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  m_vectItems[iIndex].m_dwItemData = dwData;

  return 0;

}



/*-GetItemData----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

GR::up CPropertyStack::GetItemData( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  return m_vectItems[iIndex].m_dwItemData;

}



/*-GetCount-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CPropertyStack::GetCount() const
{

  return m_vectItems.size();

}



void CPropertyStack::DrawItem( CDC* pDC, RECT rc, tPropertyItem& Item )
{

  switch ( Item.m_Type )
  {
    case E_CHECKBOX:
      InflateRect( &rc, -2, -2 );
      if ( Item.m_iValue )
      {
        pDC->DrawText( "Yes", 3, &rc, DT_SINGLELINE | DT_VCENTER );
      }
      else
      {
        pDC->DrawText( "No", 2, &rc, DT_SINGLELINE | DT_VCENTER );
      }
      break;
    case E_COMBO:
      if ( (size_t)Item.m_iValue >= Item.m_vectItems.size() )
      {
        return;
      }
      InflateRect( &rc, -2, -2 );
      pDC->DrawText( Item.m_vectItems[Item.m_iValue].first.c_str(), (int)Item.m_vectItems[Item.m_iValue].first.length(), &rc, DT_SINGLELINE | DT_VCENTER );
      break;
    case E_STATIC_TEXT:
    case E_TEXT:
    case E_NUMBER:
    case E_FILE:
      InflateRect( &rc, -2, -2 );
      pDC->DrawText( Item.m_vectItems.front().first.c_str(), (int)Item.m_vectItems.front().first.length(), &rc, DT_SINGLELINE | DT_VCENTER );
      break;
    case E_COLOR:
      {
        COLORREF    oldRGB = pDC->GetBkColor();

        RECT    rcColor = rc;

        InflateRect( &rcColor, -3, -3 );
        rcColor.right = rcColor.left + 2 * ( rcColor.bottom - rcColor.top );

        pDC->SelectObject( GetStockObject( BLACK_PEN ) );
        pDC->Rectangle( &rcColor );
        InflateRect( &rcColor, -1, -1 );

        pDC->FillSolidRect( &rcColor, RGB( ( Item.m_iValue & 0xff0000 ) >> 16,
                                           ( Item.m_iValue & 0x00ff00 ) >>  8,
                                             Item.m_iValue & 0xff ) );

        rc.left = rcColor.right + 10;
        pDC->SetBkMode( TRANSPARENT );
        pDC->DrawText( Item.m_vectItems.front().first.c_str(), (int)Item.m_vectItems.front().first.length(), &rc, DT_SINGLELINE | DT_VCENTER );
        pDC->SetBkColor( oldRGB );
      }
      break;
  }

}



void CPropertyStack::OnPaint()
{

  CPaintDC  dcPaint( this );

  CDC       dc;

  RECT    rcClient,
          rc;

  GetClientRect( &rcClient );

  if ( !dc.CreateCompatibleDC( &dcPaint ) )
  {
    OutputDebugString( "CreateCompatibleDC failed\n" );
  }

  HBITMAP   hbm = CreateCompatibleBitmap( dcPaint.GetSafeHdc(), rcClient.right - rcClient.left, rcClient.bottom - rcClient.top ),
            hbmOld;

  hbmOld = (HBITMAP)dc.SelectObject( hbm );



  HFONT hOldFont = (HFONT)SelectObject( dc.GetSafeHdc(), m_hFont );

  DRAWITEMSTRUCT    dis;

  dis.CtlID = GetDlgCtrlID();
  dis.CtlType = ODT_LISTBOX;
  dis.hDC = dc.GetSafeHdc();
  dis.hwndItem = GetSafeHwnd();

  // Hintergrund des Bars
  if ( IsWindowEnabled() )
  {
    dc.FillSolidRect( &rcClient, GetSysColor( COLOR_BTNFACE ) );
    dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
  }
  else
  {
    dc.FillSolidRect( &rcClient, GetSysColor( COLOR_INACTIVEBORDER ) );
    dc.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
  }

  dc.SetBkMode( TRANSPARENT );

  // die einzelnen Items darstellen
  for ( size_t i = m_iOffset; i < m_vectItems.size(); i++ )
  {
    if ( GetLineRect( i, &rc ) != LB_ERR )
    {
      RECT    rc2 = rc;

      rc2.right = 100;

      tPropertyItem&    Item = m_vectItems[i];

      dc.FillSolidRect( &rc, GetSysColor( COLOR_BTNFACE ) );
      dc.DrawText( Item.m_strDescription.c_str(), (int)Item.m_strDescription.length(), &rc2,
                   DT_SINGLELINE | DT_VCENTER );
      if ( ( Item.m_Type == E_STATIC_TEXT )
      &&   ( Item.m_iValue ) )
      {
        RECT    rc2;
        rc2 = rc;
        OffsetRect( &rc2, 1, 0 );
        dc.DrawText( Item.m_strDescription.c_str(), (int)Item.m_strDescription.length(), &rc2,
                    DT_SINGLELINE | DT_VCENTER );
      }

      rc.left = rc2.right;

      if ( Item.m_Type == E_STATIC_TEXT )
      {
        dc.FillSolidRect( &rc, GetSysColor( COLOR_BTNFACE ) );
      }
      else
      {
        dc.FillSolidRect( &rc, GetSysColor( COLOR_WINDOW ) );
      }
      DrawItem( &dc, rc, m_vectItems[i] );

      // Focus-Rahmen
      if ( ( i == m_iFocusedItem )
      &&   ( m_vectItems[i].m_Type != E_STATIC_TEXT )
      &&   ( m_bHasFocus ) )
      {
        dc.DrawFocusRect( &rc );
      }
    }
  }

  SelectObject( dc.GetSafeHdc(), hOldFont );

  if ( !dcPaint.BitBlt( 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                  &dc, 0, 0, SRCCOPY ) )
  {
    OutputDebugString( "BitBlt failed\n" );
  }

  dc.SelectObject( hbmOld );

  DeleteObject( hbm );

}



void CPropertyStack::EditItem( size_t iItem )
{

  if ( iItem >= m_vectItems.size() )
  {
    return;
  }
  if ( ( m_bEditingItem )
  ||   ( m_bEditingLabel ) )
  {
    return;
  }

  if ( NotifyParent( NM_BEGIN_EDIT, iItem ) )
  {
    // das Parent will nicht
    return;
  }

  ScrollIntoView( iItem );

  m_bEditingItem = true;

  tPropertyItem&   Item = m_vectItems[iItem];

  m_iEditItem = iItem;

  RECT    rc;

  GetItemRect( iItem, &rc );

  switch ( Item.m_Type )
  {
    case E_CUSTOM:
      NotifyParent( NM_EDITING, m_iEditItem );
      NotifyParent( NM_END_EDIT, m_iEditItem );
      NotifyParent( NM_ITEM_CHANGED, m_iEditItem );
      break;
    case E_CHECKBOX:
      Item.m_iValue = !!!Item.m_iValue;
      InvalidateItem( iItem );
      m_bEditingItem = false;
      NotifyParent( NM_END_EDIT, m_iEditItem );
      NotifyParent( NM_ITEM_CHANGED, m_iEditItem );
      break;
    case E_COMBO:
      {
        m_ComboPopup.Create( WS_BORDER | CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, rc, this, 1000 );
        std::vector<tPropertyItem::tComboPair>::iterator    it( Item.m_vectItems.begin() );
        while ( it != Item.m_vectItems.end() )
        {
          m_ComboPopup.SetItemData( m_ComboPopup.AddString( it->first.c_str() ), it->second );

          ++it;
        }
        m_ComboPopup.SetFont( CFont::FromHandle( m_hFont ) );
        m_ComboPopup.SetCurSel( Item.m_iValue );
        m_ComboPopup.SetFocus();
        m_ComboPopup.ShowDropDown();
        m_ComboPopup.ShowWindow( SW_SHOW );
      }
      break;
    case E_TEXT:
      m_EditPopup.Create( WS_BORDER | ES_WANTRETURN | WS_CHILD, rc, this, 1000 );
      m_EditPopup.SetWindowText( Item.m_vectItems.front().first.c_str() );
      m_EditPopup.SetFont( CFont::FromHandle( m_hFont ) );
      m_EditPopup.SetFocus();
      m_EditPopup.SetSel( 0, -1 );
      m_EditPopup.ShowWindow( SW_SHOW );
      break;
    case E_NUMBER:
      m_EditPopup.Create( WS_BORDER | ES_WANTRETURN | WS_CHILD, rc, this, 1000 );
      m_EditPopup.SetWindowText( Item.m_vectItems.front().first.c_str() );
      m_EditPopup.SetFont( CFont::FromHandle( m_hFont ) );
      m_EditPopup.SetFocus();
      m_EditPopup.SetSel( 0, -1 );
      m_EditPopup.ShowWindow( SW_SHOW );
      break;
    case E_COLOR:
      {
        CColorDialog   dlgColor( RGB( ( Item.m_iValue & 0xff0000 ) >> 16,
                                      ( Item.m_iValue & 0x00ff00 ) >>  8,
                                        Item.m_iValue & 0xff ),
                                 CC_FULLOPEN,
                                 this );

        if ( dlgColor.DoModal() == IDOK )
        {
          Item.m_iValue =   ( ( dlgColor.GetColor() & 0xff0000 ) >> 16 )
                          + ( ( dlgColor.GetColor() & 0x00ff00 ) )
                          + ( ( dlgColor.GetColor() & 0x0000ff ) << 16 );
          char    szDummy[MAX_PATH];
          wsprintf( szDummy, "#%02X%02X%02X",
                  ( Item.m_iValue & 0xff0000 ) >> 16,
                  ( Item.m_iValue & 0x00ff00 ) >>  8,
                  ( Item.m_iValue & 0x0000ff )       );
          Item.m_vectItems[0].first = szDummy;
          InvalidateItem( iItem );
        }
        m_bEditingItem = false;
        NotifyParent( NM_END_EDIT, m_iEditItem );
        NotifyParent( NM_ITEM_CHANGED, m_iEditItem );
      }
      break;
    case E_FILE:
      {
        CFileDialogEx   dlgFile( TRUE, "", Item.m_vectItems.front().first.c_str(), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, Item.m_vectItems[1].first.c_str(), this );

        if ( dlgFile.DoModal() )
        {
          Item.m_vectItems[0].first = dlgFile.GetPathName();
          InvalidateItem( iItem );

          NotifyParent( NM_ITEM_CHANGED, iItem );
        }
        m_bEditingItem = false;
      }
      break;
    default:
      m_bEditingItem = false;
      break;
  }

}



void CPropertyStack::EditLabel( size_t iItem )
{

  if ( iItem >= m_vectItems.size() )
  {
    return;
  }
  if ( ( m_bEditingItem )
  ||   ( m_bEditingLabel ) )
  {
    return;
  }

  if ( NotifyParent( NM_BEGIN_LABEL_EDIT, iItem ) )
  {
    // das Parent will nicht
    return;
  }

  ScrollIntoView( iItem );

  m_bEditingLabel = true;

  tPropertyItem&   Item = m_vectItems[iItem];

  m_iEditItem = iItem;

  RECT    rc;

  GetItemRect( iItem, &rc );

  rc.left = 0;
  rc.right = 100;

  m_EditPopup.Create( WS_BORDER | ES_WANTRETURN | WS_CHILD | ES_AUTOHSCROLL, rc, this, 1000 );
  m_EditPopup.SetWindowText( Item.m_strDescription.c_str() );
  m_EditPopup.SetFont( CFont::FromHandle( m_hFont ) );
  m_EditPopup.SetFocus();
  m_EditPopup.SetSel( 0, -1 );
  m_EditPopup.ShowWindow( SW_SHOW );

}



void CPropertyStack::OnLButtonDown(UINT nFlags, CPoint point)
{

  SetFocus();
  if ( !m_bEditingItem )
  {
  	DoLButtonDown( point );
  }

	CWnd::OnLButtonDown(nFlags, point);

}


void CPropertyStack::OnLButtonDblClk(UINT nFlags, CPoint point)
{

  SetFocus();
  int   iItem = (int)ItemFromPoint( point );
  if ( iItem != LB_ERR )
  {
    CWnd::OnLButtonDblClk(nFlags, point);

    if ( ( point.x < 100 )
    &&   ( m_vectItems[iItem].m_bLabelEditable ) )
    {
      // Label editieren
      EditLabel( iItem );
    }
    else
    {
      EditItem( iItem );
    }
    return;
  }
	//DoLButtonDown( point );

	CWnd::OnLButtonDblClk(nFlags, point);

}



void CPropertyStack::DoLButtonDown( CPoint &point )
{

  BOOL    bOutside;

  size_t iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    return;
  }
  SetCurSel( iItem );

}



size_t CPropertyStack::ItemFromPoint( CPoint pt, BOOL& bOutside ) const
{

  RECT rcClient;


  GetClientRect( &rcClient );

  bOutside = !PtInRect( &rcClient, pt );
  if ( bOutside )
  {
    return LB_ERR;
  }

  size_t    iItem = pt.y / m_iItemHeight;

  size_t iResult = m_iOffset + iItem;

  // da ist kein Item mehr
  if ( iResult >= m_vectItems.size() )
  {
    iResult = LB_ERR;
  }

  return iResult;

}



size_t CPropertyStack::ItemFromPoint( CPoint pt ) const
{

  BOOL    bOutSide;

  return ItemFromPoint( pt, bOutSide );

}



void CPropertyStack::InvalidateItem( size_t nIndex )
{

  RECT    rc;

  if ( GetItemRect( nIndex, &rc ) == LB_ERR )
  {
    return;
  }
  InvalidateRect( &rc, FALSE );

}



int CPropertyStack::GetLineRect( size_t nIndex, LPRECT lpRect ) const
{

  memset( lpRect, 0, sizeof( RECT ) );

  if ( nIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  RECT    rcClient;


  GetClientRect( &rcClient );

  if ( ( rcClient.right == 0 )
  ||   ( rcClient.bottom == 0 ) )
  {
    return LB_ERR;
  }


  lpRect->left = 0;
  lpRect->right = rcClient.right;
  lpRect->top = LONG( ( nIndex - m_iOffset ) * m_iItemHeight );
  lpRect->bottom = lpRect->top + m_iItemHeight;

  return 0;

}



int CPropertyStack::GetItemRect( size_t nIndex, LPRECT lpRect ) const
{

  int iResult = GetLineRect( nIndex, lpRect );

  if ( iResult == LB_ERR )
  {
    return iResult;
  }

  lpRect->left = 100;
  return iResult;

}



UINT CPropertyStack::OnGetDlgCode()
{

	return DLGC_WANTALLKEYS;

}



BOOL CPropertyStack::OnEraseBkgnd( CDC* pDC )
{

  return TRUE;

}



void CPropertyStack::ScrollIntoView( size_t iItem )
{

  if ( iItem == -1 )
  {
    iItem = m_iSelectedItem;
  }
  if ( iItem == -1 )
  {
    // witzisch
    return;
  }
  while ( iItem < m_iOffset )
  {
    m_iOffset--;
  }
  RECT    rc;
  GetClientRect( &rc );
  int   iVisibleItems = rc.bottom / m_iItemHeight;
  while ( iItem >= m_iOffset + iVisibleItems )
  {
    m_iOffset++;
  }

  SetScrollPos( SB_VERT, (int)m_iOffset, TRUE );

  POINT   ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

  Invalidate();

}



void CPropertyStack::SetCurSel( size_t iItem )
{

  if ( iItem >= m_vectItems.size() )
  {
    iItem = -1;
  }
  if ( m_iSelectedItem != iItem )
  {
    m_iSelectedItem = iItem;
    m_iFocusedItem = iItem;
    ScrollIntoView();
    Invalidate();

    NotifyParent( NM_ITEM_SELECTED, m_iSelectedItem );
  }

}



size_t CPropertyStack::GetCurSel( ) const
{

  if ( m_iSelectedItem == -1 )
  {
    return LB_ERR;
  }
  return m_iSelectedItem;

}



void CPropertyStack::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

  if ( m_iFocusedItem != -1 )
  {
    if ( nChar == VK_UP )
    {
      if ( m_iFocusedItem > 0 )
      {
        m_iFocusedItem--;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_DOWN )
    {
      if ( m_iFocusedItem + 1 < m_vectItems.size() )
      {
        m_iFocusedItem++;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_HOME )
    {
      if ( m_iFocusedItem > 0 )
      {
        m_iFocusedItem = 0;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_END )
    {
      if ( m_iFocusedItem + 1 < m_vectItems.size() )
      {
        m_iFocusedItem = m_vectItems.size() - 1;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_PRIOR )
    {
      if ( m_iFocusedItem >= m_iVisibleItems )
      {
        m_iFocusedItem -= m_iVisibleItems;
      }
      else
      {
        m_iFocusedItem = 0;
      }
      ScrollIntoView( m_iFocusedItem );
      Invalidate();
    }
    else if ( nChar == VK_NEXT )
    {
      if ( m_iFocusedItem + m_iVisibleItems < m_vectItems.size() )
      {
        m_iFocusedItem += m_iVisibleItems;
      }
      else
      {
        m_iFocusedItem = m_vectItems.size() - 1;
      }
      ScrollIntoView( m_iFocusedItem );
      Invalidate();
    }
    else if ( nChar == VK_RETURN )
    {
      EditItem( m_iFocusedItem );
    }
    /*
    else if ( m_vectItems[m_iFocusedItem].m_Type == E_TEXT )
    {
      EditItem( m_iFocusedItem );

      _AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();

      m_EditPopup.SendMessage( pThreadState->m_lastSentMsg.message,
		                           pThreadState->m_lastSentMsg.wParam,
                               pThreadState->m_lastSentMsg.lParam );
      return;
    }
    */
  }

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

}



/*-AdjustScrollBars-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CPropertyStack::AdjustScrollBars()
{

  RECT    rc;

  GetClientRect( &rc );

  int   iVisibleItems = rc.bottom / m_iItemHeight;


  if ( iVisibleItems > (int)m_vectItems.size() )
  {
    // gibt nichts zu scrollen
    SetScrollRange( SB_VERT, 0, 0 );
    return;
  }

  SetScrollRange( SB_VERT, 0, int( m_vectItems.size() - iVisibleItems ) );

}



void CPropertyStack::OnSize( UINT nType, int cx, int cy )
{

  SetItemHeight( m_iItemHeight );

  AdjustScrollBars();
  Invalidate();

}



void CPropertyStack::ResetContent()
{

  m_vectItems.clear();

  m_iOffset         = 0;
  m_iSelectedItem   = -1;
  m_iFocusedItem    = -1;

  AdjustScrollBars();
  Invalidate();

}



void CPropertyStack::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{

  int     iMin,
          iMax;


  GetScrollRange( SB_VERT, &iMin, &iMax );
  if ( iMax == 0 )
  {
    return;
  }

  RECT    rc;

  GetClientRect( &rc );

  size_t  iVisibleItems = rc.bottom / m_iItemHeight;

  size_t  iPageItems = ( iVisibleItems * 3 / 4 );

  size_t  iOldOffset = m_iOffset;

	switch ( nSBCode )
	{
	  case SB_LEFT:
      if ( m_iOffset > 0 )
      {
        m_iOffset = 0;
	  	  Invalidate();
      }
		  break;
	  case SB_LINELEFT:
      if ( m_iOffset > 0 )
      {
        m_iOffset--;
        if ( m_iOffset < 0 )
        {
          m_iOffset = 0;
        }
  		  Invalidate();
      }
		  break;
	  case SB_PAGELEFT:
      if ( m_iOffset )
      {
        if ( m_iOffset >= iPageItems )
        {
          m_iOffset -= iPageItems;
        }
        else
        {
          m_iOffset = 0;
        }
  		  Invalidate();
      }
		  break;
	  case SB_LINERIGHT:
      m_iOffset++;
      if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
      }
		  Invalidate();
		  break;
	  case SB_PAGERIGHT:
      m_iOffset += iPageItems;
      if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_iOffset = m_vectItems.size() - iVisibleItems;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      if ( nPos != m_iOffset )
      {
        m_iOffset = nPos;
		    Invalidate();
      }
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}

  if ( iOldOffset != m_iOffset )
  {
	  SetScrollPos( SB_VERT, (int)m_iOffset, TRUE );
  }

	CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

  POINT   ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

}

void CPropertyStack::OnMouseMove(UINT nFlags, CPoint point)
{

  /*
  BOOL      bOutside = FALSE;

  size_t iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    if ( m_iMouseOverItem != -1 )
    {
      m_iMouseOverItem = -1;
      Invalidate();
    }
  }
  else
  {
    if ( m_iMouseOverItem != iItem )
    {
      m_iMouseOverItem = iItem;
      Invalidate();
    }
  }
  if ( m_iMouseOverItem != -1 )
  {
    if ( m_iTimerID == -1 )
    {
      //SetTimer( m_iTimerID, 50, NULL );
    }
  }
  */

  CWnd::OnMouseMove(nFlags, point);
}

void CPropertyStack::OnTimer(UINT nIDEvent)
{

  if ( nIDEvent == m_iTimerID )
  {
    POINT   ptMousePos;

    GetCursorPos( &ptMousePos );
    ScreenToClient( &ptMousePos );

    RECT    rc;

    GetClientRect( &rc );

    if ( !PtInRect( &rc, ptMousePos ) )
    {
      Invalidate();
      KillTimer( m_iTimerID );
      m_iTimerID = -1;
    }
  }

  CWnd::OnTimer(nIDEvent);
}





BOOL CPropertyStack::PreTranslateMessage(MSG* pMsg)
{

  if ( pMsg->hwnd == m_EditPopup.GetSafeHwnd() )
  {
    if ( pMsg->message == WM_CHAR )
    {
      if ( m_vectItems[m_iEditItem].m_Type == E_NUMBER )
      {
        if ( ( pMsg->wParam >= '0' )
        &&   ( pMsg->wParam <= '9' )
        ||   ( pMsg->wParam == '.' )
        ||   ( pMsg->wParam == VK_BACK )
        ||   ( pMsg->wParam == '-' ) )
        {
        }
        else
        {
          return TRUE;
        }
      }
    }
    if ( ( pMsg->message == WM_KEYDOWN )
    &&   ( pMsg->wParam == VK_RETURN ) )
    {
      CString   cstrGnu;

      m_EditPopup.GetWindowText( cstrGnu );

      if ( m_bEditingLabel )
      {
        m_vectItems[m_iEditItem].m_strDescription = LPCSTR( cstrGnu );

        NotifyParent( NM_END_LABEL_EDIT, m_iEditItem );
        NotifyParent( NM_LABEL_CHANGED, m_iEditItem );
      }
      else
      {
        m_vectItems[m_iEditItem].m_vectItems[0].first = LPCSTR( cstrGnu );
        if ( m_vectItems[m_iEditItem].m_Type == E_NUMBER )
        {
          m_vectItems[m_iEditItem].m_iValue = atoi( cstrGnu );
        }
        NotifyParent( NM_END_EDIT, m_iEditItem );
        NotifyParent( NM_ITEM_CHANGED, m_iEditItem );
      }

      InvalidateItem( m_iEditItem );


      m_EditPopup.DestroyWindow();

      m_bEditingItem = false;
      m_bEditingLabel = false;
      m_iEditItem = -1;
      return TRUE;
    }
    else if ( ( pMsg->message == WM_KEYDOWN )
    &&        ( pMsg->wParam == VK_ESCAPE ) )
    {
      if ( m_bEditingLabel )
      {
        NotifyParent( NM_END_LABEL_EDIT, m_iEditItem );
      }
      else
      {
        NotifyParent( NM_END_EDIT, m_iEditItem );
      }

      m_EditPopup.DestroyWindow();

      m_bEditingItem = false;
      m_bEditingLabel = false;
      m_iEditItem = -1;
      return TRUE;
    }
  }

  return CWnd::PreTranslateMessage(pMsg);
}

void CPropertyStack::OnLButtonUp(UINT nFlags, CPoint point)
{

  if ( m_bEditingItem )
  {
    return;
  }

  CWnd::OnLButtonUp(nFlags, point);
}


BOOL CPropertyStack::OnCommand(WPARAM wParam, LPARAM lParam)
{

  if ( (HWND)lParam == m_EditPopup.GetSafeHwnd() )
  {
    if ( HIWORD( wParam ) == EN_KILLFOCUS )
    {
      m_EditPopup.DestroyWindow();

      if ( m_bEditingLabel )
      {
        NotifyParent( NM_END_LABEL_EDIT, m_iEditItem );
      }
      else
      {
        NotifyParent( NM_END_EDIT, m_iEditItem );
      }


      m_bEditingLabel = false;
      m_bEditingItem = false;
      m_iEditItem = -1;
      return TRUE;
    }
  }
  else if ( (HWND)lParam == m_ComboPopup.GetSafeHwnd() )
  {
    if ( HIWORD( wParam ) == CBN_SELCHANGE )
    {
      m_vectItems[m_iEditItem].m_iValue = m_ComboPopup.GetCurSel();
      NotifyParent( NM_ITEM_CHANGED, m_iEditItem );
    }
    else if ( HIWORD( wParam ) == CBN_KILLFOCUS )
    {
      NotifyParent( NM_END_EDIT, m_iEditItem );
      m_ComboPopup.DestroyWindow();
      m_bEditingItem = false;
      m_iEditItem = -1;
      return TRUE;
    }
  }

  return CWnd::OnCommand(wParam, lParam);
}

void CPropertyStack::OnKillFocus(CWnd* pNewWnd)
{

  m_bHasFocus = false;

  CWnd::OnKillFocus( pNewWnd );

}



void CPropertyStack::OnSetFocus(CWnd* pOldWnd)
{

  m_bHasFocus = true;
  CWnd::OnSetFocus(pOldWnd);

}



void CPropertyStack::SetItemValue( size_t iIndex, int iValue )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return;
  }
  tPropertyItem&  Item = m_vectItems[iIndex];

  if ( Item.m_Type == E_NUMBER )
  {
    Item.m_iValue = iValue;
    Item.m_vectItems[0].first = CMisc::printf( "%d", iValue );

    InvalidateItem( iIndex );
  }

}



BOOL CPropertyStack::NotifyParent( WPARAM wParam, LPARAM lParam )
{

  CWnd* pParent = GetParent();
  if ( pParent == NULL )
  {
    return FALSE;
  }

  return (BOOL)pParent->SendMessage( m_dwNotifyMessage, wParam, lParam );

}



void CPropertyStack::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{

  if ( m_iFocusedItem != -1 )
  {
    if ( m_vectItems[m_iFocusedItem].m_Type == E_TEXT )
    {
      EditItem( m_iFocusedItem );

      _AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();

      m_EditPopup.SendMessage( pThreadState->m_lastSentMsg.message,
		                           pThreadState->m_lastSentMsg.wParam,
                               pThreadState->m_lastSentMsg.lParam );
      return;
    }
  }

  CWnd::OnChar(nChar, nRepCnt, nFlags);
}



void CPropertyStack::SetLabelEditable( size_t iIndex, bool bEditable )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return;
  }
  m_vectItems[iIndex].m_bLabelEditable = bEditable;

}