#ifndef GUI_IMAGELIST_H
#define GUI_IMAGELIST_H



#include <Controls\AbstractListBox.h>

#include <Xtreme/XTextureSection.h>

#include "GUIComponent.h"
#include "GUIScrollbar.h"



class GUIImageList : public AbstractListBox<GUIComponent,GUIScrollBar>
{

  public:

    DECLARE_CLONEABLE( GUIImageList, "ImageList" );


    GUIImageList( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0, GR::u32 lbType = LCS_SINGLE_SELECT | LCS_MULTI_COLUMN );


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void          AddString( const XTextureSection& TexSec, GR::u32 dwItemData = 0 );
    void                  SetItemImage( const size_t iIndex, const XTextureSection& TexSec );

};


#endif // __GUI_IMAGELIST_H



