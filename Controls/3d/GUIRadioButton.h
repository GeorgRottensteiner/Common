#ifndef __GUI_RADIOBUTTON_H__
#define __GUI_RADIOBUTTON_H__
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#pragma warning ( disable:4786 )

#include <string>
#include <vector>

#include <Controls\AbstractRadioButton.h>
#include "GUIComponent.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIRadioButton : public CAbstractRadioButton<CGUIComponent>
{

  protected:

    DWORD         m_dwPushedColor,
                  m_dwMouseOverColor;


  public:

    CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId = 0 );
    CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );


    virtual ICloneAble*       Clone();
};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_RADIOBUTTON_H__



