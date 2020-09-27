#ifndef __GUI_BUTTON_H__
#define __GUI_BUTTON_H__
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <Controls\AbstractButton.h>
#include "GUIComponent.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIButton : public CAbstractButton<CGUIComponent>
{

  protected:

    GR::u32                   m_dwMouseOverColor;


  public:

    DECLARE_CLONEABLE( CGUIButton, "Button" )

    CGUIButton( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 dwId = 0 );
    CGUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

};


#endif // __GUI_BUTTON_H__



