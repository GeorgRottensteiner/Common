#ifndef __GUI_LABEL_H__
#define __GUI_LABEL_H__
/*--------------------+-------------------------------------------------------+
 | Programmname       : GUILabel                                              |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUILabel : public CAbstractLabel<CGUIComponent>
{

  protected:


  public:


    CGUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID = 0 );
    CGUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GUI::AlignmentFlagType dwFlags, const char* szCaption, GR::u32 dwID = 0 );


    virtual void        DisplayOnPage( CD3DViewer* pViewer, int iXOffset = 0, int iOffsetY = 0 );

    virtual ICloneAble* Clone();

};


#endif // __GUI_LABEL_H__



