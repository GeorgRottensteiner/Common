#ifndef __GUI_LISTBOX_H__
#define __GUI_LISTBOX_H__
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

#include <Controls\AbstractListBox.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIListBox : public CAbstractListBox<CGUIComponent,CGUIScrollBar>
{

  protected:


  public:


    CGUIListBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId = 0, GR::u32 lbfType = LCS_DEFAULT );


    virtual void          DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY );

    virtual ICloneAble*   Clone();

};


#endif // __GUI_LISTBOX_H__



