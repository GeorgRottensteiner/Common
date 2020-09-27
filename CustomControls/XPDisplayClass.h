#ifndef GR_UI_XP_DISPLAY_CLASS_H
#define GR_UI_XP_DISPLAY_CLASS_H

#include <string>

#include <windows.h>

#include <ControlHelper/CWnd.h>

#include <GR/GRTypes.h>

#include "ClassicDisplayClass.h"



class CXPDisplayClass : public CClassicDisplayClass
{

  protected:


  public:

    CXPDisplayClass();

    virtual void          PaintMenuItem( HDC dc, const RECT& rc, tGRUIMenuItem& MenuItem, bool bPaintBackground );
    virtual void          PaintButton( HDC hdc, const RECT& rc, const char* szText, bool bMouseOver, bool bPushed, bool bPaintBackground );

};


#endif // GR_UI_XP_DISPLAY_CLASS_H