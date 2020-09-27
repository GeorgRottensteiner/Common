#ifndef GUI_IMAGE_H
#define GUI_IMAGE_H



#include <string>
#include <vector>

#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



class GUIImage : public AbstractLabel<GUIComponent>
{

  protected:

    XTextureSection     m_Image;


  public:

    DECLARE_CLONEABLE( GUIImage, "Image" )


    GUIImage( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 ID = 0 );


    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

    void                SetImage( const XTextureSection& tsImage );


    virtual void        ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );


};


#endif // GUI_IMAGE_H



