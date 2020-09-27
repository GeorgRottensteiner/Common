#ifndef INCLUDE_CSTATIC_H
#define INCLUDE_CSTATIC_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CStatic : public CWnd
  {

    protected:

    bool                m_bSelfDrawn;

    COLORREF            m_rgbTextColor,
      m_rgbBackground;

    public:

    CStatic();
    virtual ~CStatic();


    HICON               SetIcon( HICON hIcon );
    HBITMAP             SetBitmap( HBITMAP hBitmap );

    HICON               GetIcon() const;

    virtual BOOL        PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    virtual void        OnPaint( HDC hdc );

    void                SetTextColor( COLORREF rgbColor );

    void                SetSelfDraw( bool bDraw = true );

  };

}

#endif //__INCLUDE_CSTATIC_H__



