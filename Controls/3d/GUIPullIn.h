#ifndef GUI_PULLIN_H
#define GUI_PULLIN_H
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Controls\AbstractPullIn.h>

#include "GUIComponent.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIPullIn : public CAbstractPullIn<CGUIComponent>
{

  protected:


  public:


    CGUIPullIn( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dfType, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iXOffset = 0, int iYOffset = 0 );
    virtual void              DisplayNonClientOnPage( CD3DViewer* pViewer, int iXOffset = 0, int iYOffset = 0 );


    virtual ICloneAble*       Clone();
};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // GUI_PULLIN_H



