#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractListBox.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUIListBox : public AbstractListBox<GUIComponent,GUIScrollBar>
{


  public:

    DECLARE_CLONEABLE( GUIListBox, "ListBox" )


    GUIListBox( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0, GR::u32 ListBoxFlags = LCS_DEFAULT );


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_LISTBOX_H



