#ifndef GUI_LISTCONTROL_H
#define GUI_LISTCONTROL_H



#include <string>
#include <vector>

#include <Controls\AbstractListCtrl.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUIListControl : public AbstractListCtrl<GUIComponent, GUIScrollBar>
{

  public:

    DECLARE_CLONEABLE( GUIListControl, "ListCtrl" )


    GUIListControl( int NewX = 0, int NewY = 0, int NewWidth = 20, int NewHeight = 20, GR::u32 ListControlFlags = LCS_DEFAULT, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void              SetTextureSection( const GUI::eBorderType Border, const XTextureSection& TexSection );

};


#endif // GUI_LISTCONTROL_H



