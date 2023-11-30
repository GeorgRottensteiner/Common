#ifndef GUI_IMAGELIST_H
#define GUI_IMAGELIST_H



#include <string>
#include <vector>

#include <Controls\AbstractListBox.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GR::Graphic::GFXPage;



class GUIImageList : public AbstractListBox<GUIComponent,GUIScrollbar>
{

  public:

    DECLARE_CLONEABLE( GUIImageList, "GUIImageList" )


    GUIImageList( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0, GR::u32 lbType = LCS_SINGLE_SELECT | LCS_MULTI_COLUMN );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );


    virtual void        AddString( GR::Graphic::Image* pImage, GR::up dwItemData = 0 );

};


#endif // GUI_IMAGELIST_H



