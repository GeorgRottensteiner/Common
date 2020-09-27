#ifndef GUI_PROGRESSBAR_H
#define GUI_PROGRESSBAR_H



#include <string>
#include <vector>

#include <Controls\AbstractProgressBar.h>
#include "GUIComponent.h"



class GUIProgressBar : public GUI::AbstractProgressBar<GUIComponent>
{

  protected:

    XTextureSection     m_tsFillImage;


  public:

    DECLARE_CLONEABLE( GUIProgressBar, "GUI.ProgressBar" )


    GUIProgressBar( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 ID = 0 );


    virtual void        DisplayOnPage( XRenderer* pRenderer );

    void                SetImage( const XTextureSection& tsFillImage );

};


#endif // GUI_PROGRESSBAR_H



