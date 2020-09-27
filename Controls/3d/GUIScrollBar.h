#ifndef __GUI_SCROLLBAR_H__
#define __GUI_SCROLLBAR_H__
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Controls\AbstractScrollBar.h>
#include "GUIComponent.h"
#include "GUIButton.h"
#include "GUISlider.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIScrollBar : public CAbstractScrollbar<CGUIComponent, CGUIButton, CGUISlider>
{

  public:

    DECLARE_CLONEABLE( CGUIScrollBar, "Scrollbar" )


    CGUIScrollBar( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, ScrollbarFlagType sfType = SBFT_INVALID, DWORD dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

};


#endif // __GUI_SCROLLBAR_H__



