#ifndef __GUI_TEXTURED_BUTTON_H__
#define __GUI_TEXTURED_BUTTON_H__
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

#include <Controls\AbstractButton.h>
#include <DX8/TextureSection.h>

#include "GUIComponent.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUITexturedButton : public CAbstractButton<CGUIComponent>
{

  protected:

    tTextureSection           m_TextureSectionCenter;


  public:

    CGUITexturedButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId = 0 );
    CGUITexturedButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    virtual void              SetTextureSection( const tTextureSection& TexSection );

    virtual ICloneAble*       Clone();

};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_TEXTURED_BUTTON_H__



