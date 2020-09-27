#ifndef GUI_IMAGELIST_H
#define GUI_IMAGELIST_H
/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Controls\AbstractListBox.h>

#include <DX8/TextureSection.h>

#include "GUIComponent.h"
#include "GUIScrollbar.h"



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIImageList : public CAbstractListBox<CGUIComponent,CGUIScrollBar>
{

  protected:


  public:


    CGUIImageList( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId = 0, GR::u32 lbType = LCS_SINGLE_SELECT | LCS_MULTI_COLUMN );


    virtual void          DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY );


    virtual void          AddString( const tTextureSection& TexSec, GR::u32 dwItemData = 0 );
    void                  SetItemImage( const size_t iIndex, const tTextureSection& TexSec );

    virtual ICloneAble*   Clone();

};


#endif // __GUI_IMAGELIST_H



