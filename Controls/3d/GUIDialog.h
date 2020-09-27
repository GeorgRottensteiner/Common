#ifndef __GUI_DIALOG_H__
#define __GUI_DIALOG_H__
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

#include <Controls\AbstractDialog.h>
#include "GUIComponent.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIDialog : public CAbstractDialog<CGUIComponent>
{

  protected:


  public:


    CGUIDialog( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dfType, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iXOffset = 0, int iYOffset = 0 );
    virtual void              DisplayNonClientOnPage( CD3DViewer* pViewer, int iXOffset = 0, int iYOffset = 0 );


    virtual ICloneAble*       Clone();
};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_DIALOG_H__



