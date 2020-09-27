#ifndef __GUI_SLIDER_H__
#define __GUI_SLIDER_H__
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Controls\AbstractSlider.h>
#include "GUIComponent.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUISlider : public CAbstractSlider<CGUIComponent>
{

  public:

    DECLARE_CLONEABLE( CGUISlider, "Slider" )


    CGUISlider( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, SliderFlagType sftFlags = SFT_VERTICAL, DWORD dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );


};


#endif // __GUI_SLIDER_H__



