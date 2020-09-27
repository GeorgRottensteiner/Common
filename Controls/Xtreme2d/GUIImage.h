#ifndef GUI_IMAGE_H
#define GUI_IMAGE_H



#include <string>
#include <vector>

#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



class GUIImage : public AbstractLabel<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIImage, "Image" )


    GUIImage( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwID = 0 );


    virtual void        Display( GUIComponentDisplayer* pDisplayer );

    void                SetImage( const XTextureSection& tsImage );


};


#endif // GUI_IMAGE_H



