#ifndef GUI_COMBOBOX_H
#define GUI_COMBOBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractComboBox.h>
#include "GUIComponent.h"
#include "GUIListBox.h"
#include "GUIButton.h"
#include "GUIEdit.h"




class GUIComboBox : public AbstractComboBox<GUIComponent,GUIEdit,GUIButton,GUIListBox>
{


  public:

    DECLARE_CLONEABLE( GUIComboBox, "Combobox" )


    GUIComboBox( int X = 0, int Y = 0, int Width = 0, int Height = 0, GR::u32 Type = CFT_DEFAULT, GR::u32 Id = 0 );

    virtual void          SetFont( XFont* pFont );


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual void          SetTextureSection( const GUI::eBorderType Type, const XTextureSection& TexSection );

};



#endif // GUI_COMBOBOX_H



