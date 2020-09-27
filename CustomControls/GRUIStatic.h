#ifndef GR_UI_STATIC_H
#define GR_UI_STATIC_H

#include "CustomWnd.h"


class GRUIStatic : public CCustomWnd
{

  public:

    virtual void Draw( HDC hdc, const RECT& rc )
    {
      char    szText[MAX_PATH];
      GetWindowText( szText, MAX_PATH );

      m_pDisplayClass->PaintStatic( hdc, rc, szText, m_pWndBackgroundProducer == NULL );
    }

};


#endif // GR_UI_STATIC_H