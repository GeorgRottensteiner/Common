#pragma once

// CTransparentStatic
#include <ControlHelper/CStatic.h>


class CTransparentStatic : public CStatic
{

public:
	CTransparentStatic();
	virtual ~CTransparentStatic();

public:

  GR::String             m_strText;

  HWND                    m_hwndBackground;


  void                    OnPaint();

  virtual void            OnAttach();
  virtual void            DrawItem( LPDRAWITEMSTRUCT /*lpDrawItemStruct*/ );

  virtual void            DoPaint( HDC hdc );

  void                    SetWindowText( LPCTSTR lpszString );
  void                    SetAlternateBackgroundWnd( HWND hwndBackground );

  virtual LRESULT         WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

};


