#pragma once

#include <CustomControls/CustomWnd.h>



class GRUIMDIDocument;

class GRUIMDIView : public CCustomWnd
{

  public:

    GRUIMDIDocument*          m_pDocument;


    GRUIMDIView( GRUIMDIDocument* pDoc );
    virtual ~GRUIMDIView();


    virtual LRESULT           WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    virtual void              Draw( HDC hdc, const RECT& rc );
    virtual void              DrawBackground( HDC hdc, const RECT& rc );


    virtual void              OnMDIActivate();
    virtual void              OnMDIDeactivate();


  private:

    HDC                       m_hdcTemp;

    HBITMAP                   m_hbmOld;


    bool                      IsCaptionActive();

    virtual HDC               BeginNCPaint();

    virtual HDC               StartPaint();
    virtual void              StopPaint();

};
