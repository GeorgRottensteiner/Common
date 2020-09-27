#ifndef GR_UI_CLASSIC_DISPLAY_CLASS_H
#define GR_UI_CLASSIC_DISPLAY_CLASS_H

#include <string>

#include <windows.h>
#include <commctrl.h>

#include <ControlHelper/CWnd.h>

#include <GR/GRTypes.h>


class CCustomWnd;
class CGRUIMenu;
class GRUICmdManager;

struct tItemInfo
{
  enum eItemFlags
  {
    IF_DEFAULT      = 0,
    IF_HILIGHTED    = 0x00000001,
    IF_PUSHED       = 0x00000002,
    IF_DISABLED     = 0x00000004,
    IF_CHECKED      = 0x00000008,
    IF_GRAYED       = 0x00000010,
    IF_SYSMENUICON  = 0x40000000,   // das System-Menü-Icon ganz links/oben
    IF_SYSMENUBTN   = 0x80000000,   // Caption-Buttons Minimize/Maximize/Close für maximierte MDI-Childs
  };

  DWORD_PTR     dwCommandID;
  DWORD         dwButtonStyle;
  DWORD_PTR     dwExtraData;
  RECT          rcButton;
  GR::String   strText;
  HICON         hIcon;
  CGRUIMenu*    pSubMenu;

  tItemInfo() :
    dwCommandID( 0 ),
    dwButtonStyle( 0 ),
    dwExtraData( 0 ),
    strText( "" ),
    hIcon( NULL ),
    pSubMenu( NULL )
  {
    SetRectEmpty( &rcButton );
  }
};

struct tGRUIMenuItem
{
  GR::String   strItemText;
  GR::up        dwID;
  GR::up        dwItemData;
  GR::u32       dwFlags;
  HICON         hiconUnchecked,
                hiconChecked;
  RECT          rcSize;

  tGRUIMenuItem() :
    dwID( 0 ),
    dwItemData( 0 ),
    dwFlags( 0 ),
    strItemText( "" ),
    hiconUnchecked( NULL ),
    hiconChecked( NULL )
  {
    SetRectEmpty( &rcSize );
  }

};

struct tToolBarButtonInfo
{
  enum eButtonFlags
  {
    BTN_DEFAULT     = 0,
    BTN_HILIGHTED   = 0x00000001,
    BTN_PUSHED      = 0x00000002,
    BTN_DISABLED    = 0x00000004,
    BTN_CHECKABLE   = 0x00000008,
    BTN_CHECKED     = 0x00000010,
    BTN_RADIO       = 0x00000020,
  };

  DWORD     dwCommandID;
  DWORD     dwButtonStyle;
  RECT      rcButton;

  tToolBarButtonInfo() :
    dwCommandID( 0 ),
    dwButtonStyle( 0 )
  {
    SetRectEmpty( &rcButton );
  }
};



class CClassicDisplayClass
{

  protected:

    struct tDisplayConstants
    {
      int       m_iMenuItemIndent;
      int       m_iMenuItemHeight;
      int       m_iMenuBorderWidth;
      int       m_iMenuBorderHeight;
      int       m_iMenuSeparatorHeight;
      int       m_iMenuPopupArrowWidth;
      int       m_iMenuPopupArrowHeight;
      int       m_iCheckWidth;
      int       m_iCheckHeight;
      GR::u32   m_dwMenuWindowStyles;
      int       m_iSliderSize;
      int       m_iSizeBorderWidth;
      int       m_iSizeBorderHeight;
      int       m_iCaptionHeight;
      int       m_iSmallCaptionHeight;
      int       m_iClientEdgeWidth;
      int       m_iClientEdgeHeight;
      int       m_iToolBarGripperWidth;
      int       m_iToolBarGripperHeight;
    };

    HICON                 m_hIconCheck,
                          m_hIconPopupArrow;


  public:

    tDisplayConstants     m_DisplayConstants;

    GRUICmdManager*       m_pCmdManager;


    CClassicDisplayClass();
    virtual ~CClassicDisplayClass();


    void                  SetCommandManager( GRUICmdManager* pCmdManager );

    void                  FillSolidRect( HDC hdc, const RECT* pRC, COLORREF clr );
    void                  FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr );

    void                  Draw3dRect( HDC dc, int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight );
    void                  Draw3dRect( HDC dc, const RECT& rc, COLORREF clrTopLeft, COLORREF clrBottomRight);


    virtual void          DrawBackground( HDC hdc, const RECT& rcRedraw );

    virtual void          PaintIcon( HDC hdc, const RECT& rc, HICON hIcon );
    virtual void          PaintIconSized( HDC hdc, const RECT& rc, HICON hIcon, int iWidth, int iHeight );
    virtual void          PaintBitmap( HDC hdc, const RECT& rc, HBITMAP hbm );
    virtual void          PaintEdit( HDC hdc, const RECT& rc, const GR::String& strText, bool bTransparent, bool bFocused, bool bMultiLine, int iFirstVisibleChar, int iSelStart, int iSelEnd );

    virtual void          PaintMenuItem( HDC dc, const RECT& rc, int iMenuIndention, tGRUIMenuItem& MenuItem, bool bPaintBackground );
    virtual void          PaintMenuBarItem( HDC hdc, bool bHorizontal, const tItemInfo& ItemInfo );

    virtual void          PaintButton( HDC dc, const RECT& rcItem, const char* szText, bool bMouseOver, bool bPushed, bool bPaintBackground, bool bFocused );
    virtual void          PaintCheckBox( HDC hdc, const RECT& rc, const char* szText, bool bMouseOver, bool bPushed, bool bChecked, bool bPaintBackground );
    virtual void          PaintSlider( HDC hdc, const RECT& rc, const RECT& rcSlider, bool bMouseOver, bool bMouseOverSlider, bool bDragging, bool bPaintBackground );
    virtual void          PaintStatic( HDC hdc, const RECT& rc, const char* szText, bool bPaintBackground );

    virtual void          PaintToolBarBack( HDC hdc, const RECT& rc, bool bHorizontal );
    virtual void          PaintToolBarGripper( HDC hdc, const RECT& rc, bool bHorizontal );
    virtual void          PaintToolBarButton( HDC hdc, int iX, int iY, const tToolBarButtonInfo& BtnInfo, HIMAGELIST hImageList, int iButtonIndex );

    virtual void          PaintWindowBorder( HDC hdc, const RECT& rc, DWORD dwStyle, DWORD dwExStyle = 0 );
    virtual void          PaintWindowCaption( HDC hdc, const RECT& rc, HWND hWnd, bool bActive );
    virtual void          PaintWindowCaptionCloseButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive );
    virtual void          PaintWindowCaptionMaximizeButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive, bool IsMaximized );
    virtual void          PaintWindowCaptionMinimizeButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive, bool bIsMinimized );

    virtual void          PaintDockBarBackground( HDC hdc, const RECT& rc );

};


#endif // GR_UI_CLASSIC_DISPLAY_CLASS_H