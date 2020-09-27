#ifndef GUI_TREECONTROL_H
#define GUI_TREECONTROL_H
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

#include <Controls\AbstractTreeCtrl.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUITreeCtrl : public CAbstractTreeCtrl<CGUIComponent, CGUIScrollBar>
{

  protected:

    std::vector<tTextureSection>      m_vectSections;


  public:

    CGUITreeCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles = TCFT_DEFAULT, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    virtual int               AddImage( const tTextureSection& Section );
    virtual void              SetItemImage( TREEITEM hItem, GR::u32 dwImage );

    TREEITEM                  InsertItem( TREEITEM hItemParent, const char* szString, GR::u32 dwItemImage = 0, GR::u32 dwItemData = 0 );
    TREEITEM                  InsertItem( const char* szString, GR::u32 dwItemImage = 0, GR::u32 dwItemData = 0 );


    virtual ICloneAble*       Clone();

};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // GUI_TREECONTROL_H



