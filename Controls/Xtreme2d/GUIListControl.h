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


    GUIListControl( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 dwStyles = LCS_DEFAULT, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void              SetTextureSection( const GUI::eBorderType eType, const XTextureSection& TexSection );

};


#endif // GUI_LISTCONTROL_H



