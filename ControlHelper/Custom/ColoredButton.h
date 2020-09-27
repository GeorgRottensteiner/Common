#pragma once


#include <windows.h>
#include <ControlHelper/CButton.h>
// CColoredButton

class CColoredButton : public CButton
{



  public:

	  CColoredButton();
	  virtual ~CColoredButton();

    BOOL            OnEraseBkgnd();
    virtual void    DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

    void            SetBackground( COLORREF rgbColor, COLORREF rgbDisabledColor );


  protected:


    COLORREF                m_rgbBackground,
                            m_rgbDisabledBackground,
                            m_rgbTextColor;

};


