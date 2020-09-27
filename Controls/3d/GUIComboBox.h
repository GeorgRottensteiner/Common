#ifndef __GUI_COMBOBOX_H__
#define __GUI_COMBOBOX_H__
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

#include <Controls\AbstractComboBox.h>
#include "GUIComponent.h"
#include "GUIListBox.h"
#include "GUIButton.h"
#include "GUIEdit.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CDX8Texture;



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIComboBox : public CAbstractComboBox<CGUIComponent,CGUIEdit,CGUIButton,CGUIListBox>
{

  protected:

    CDX8Texture*          m_pTexture;


  public:


    CGUIComboBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 cfType = CFT_DEFAULT, DWORD dwId = 0 );

    virtual void          SetTexture( CDX8Texture* pTexture );

    virtual void          SetFont( CDX8Font* pFont );


    virtual void          DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY );

    virtual void          SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection );

    virtual ICloneAble*   Clone();

};



#endif // __GUI_COMBOBOX_H__



