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

    DECLARE_CLONEABLE( GUIComboBox, "ComboBox" )


    GUIComboBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 cfType = CFT_DEFAULT, DWORD dwId = 0 );

    virtual void          SetFont( X2dFont* pFont );


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual void          SetTextureSection( const GUI::eBorderType eType, const XTextureSection& TexSection );

};



#endif // GUI_COMBOBOX_H



