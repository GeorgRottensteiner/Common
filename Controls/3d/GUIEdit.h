#ifndef __GUI_EDIT_H__
#define __GUI_EDIT_H__
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

#include <Controls\AbstractEdit.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"
#include "GUISlider.h"
#include "GUIButton.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIEdit : public CAbstractEdit<CGUIComponent, CGUIScrollBar, CGUISlider, CGUIButton>
{

  protected:

    CDX8Texture*        m_pTexture;


  public:


    CGUIEdit( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 efType, GR::u32 dwId = 0 );

    virtual void        DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    virtual void        SetTexture( CDX8Texture* );


    virtual ICloneAble*       Clone();

};


#endif // __GUI_EDIT_H__



