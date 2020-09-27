#ifndef __GUI_SCREEN_H__
#define __GUI_SCREEN_H__
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

class CGUIScreen : public CGUIComponent
{

  protected:



  public:

    CGUIScreen( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );


    virtual ICloneAble*       Clone();

};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_SCREEN_H__



