#ifndef GUI_COMBOBOX_H
#define GUI_COMBOBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractComboBox.h>
#include "GUIComponent.h"
#include "GUIListBox.h"
#include "GUIButton.h"
#include "GUIEdit.h"



namespace GR
{
  namespace Graphic
  {
    class GFXPage;
  };
};



class CGUIComboBox : public CAbstractComboBox<CGUIComponent,CGUIEdit,CGUIButton,CGUIListBox>
{

  public:


    DECLARE_CLONEABLE( CGUIComboBox, "ComboBox" )


    CGUIComboBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 cfType = CFT_DEFAULT, GR::u32 dwId = 0 );


    virtual void          DisplayOnPage( GR::Graphic::GFXPage* pPage );

};



#endif // GUI_COMBOBOX_H



