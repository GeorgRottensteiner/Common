#pragma once

#include <ControlHelper/CButton.h>



class CTransparentButton : public CButton
{


  public:

	  CTransparentButton();
	  virtual ~CTransparentButton();

    void                    SetAlternateBackgroundWnd( HWND hWndBackground );

protected:


    HWND                    m_hwndBackground;

    HBITMAP                 m_hbmCheckBox;


    void                    OnPaint();

    virtual void            OnAttach();

    virtual LRESULT         WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

};


