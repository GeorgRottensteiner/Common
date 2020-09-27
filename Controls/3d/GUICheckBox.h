#ifndef __GUI_CHECKBOX_H__
#define __GUI_CHECKBOX_H__
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

#include <Controls\AbstractCheckbox.h>
#include "GUIComponent.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUICheckBox : public CAbstractCheckBox<CGUIComponent>
{

  protected:

    CDX8Texture*  m_pTexture;


  public:

    CGUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId = 0 );
    CGUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    virtual void              SetTexture( CDX8Texture* );


    virtual ICloneAble*       Clone();

};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_CHECKBOX_H__



