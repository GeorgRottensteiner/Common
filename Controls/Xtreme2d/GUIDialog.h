#ifndef GUI_DIALOG_H
#define GUI_DIALOG_H



#include <string>
#include <vector>

#include <Controls\AbstractDialog.h>
#include "GUIComponent.h"



class GUIDialog : public AbstractDialog<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIDialog, "Dialog" )


    GUIDialog( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, const GR::String& Caption = GR::String(), GR::u32 Type = 0, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );


};


#endif // GUI_DIALOG_H



