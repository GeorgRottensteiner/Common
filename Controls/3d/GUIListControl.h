#ifndef __GUI_LISTCONTROL_H__
#define __GUI_LISTCONTROL_H__
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

#include <Controls\AbstractListCtrl.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIListControl : public CAbstractListCtrl<CGUIComponent, CGUIScrollBar>
{

  public:

    DECLARE_CLONEABLE( CGUIListControl, "ListCtrl" )


    CGUIListControl( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 dwStyles = LCS_DEFAULT, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );


    virtual void              SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection );

};


#endif // __GUI_LISTCONTROL_H__



