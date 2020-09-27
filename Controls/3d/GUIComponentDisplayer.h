#ifndef GUI_COMPONENT_DISPLAYER_H
#define GUI_COMPONENT_DISPLAYER_H



#include <DX8\DX8Viewer.h>
#include <DX8/TextureSection.h>

#include <Controls/Component.h>
#include <Controls\AbstractComponentDisplayer.h>

#include "GUIComponent.h"

#include <string>
#include <vector>



class GUIComponentDisplayer : public GUI::AbstractComponentDisplayer<GUIComponent>
{

  protected:

    typedef std::vector<tTextureSection>      tVectDefaultTextureSections;

    GR::tRect                       m_rectClipping;

    tVectDefaultTextureSections     m_DefaultTextureSection;


    GUIComponentDisplayer();


  public:

    int                             m_iOffsetX,
                                    m_iOffsetY;

    CD3DViewer*                     m_pViewer;



    virtual void                    DisplayAllControls();

    virtual void                    SetClipping( int iX1, int iY1, int iX2, int iY2 );
    virtual void                    SetOffset( int iX, int iY );
    virtual void                    PushClipValues();

    void                            SetDefaultTextureSection( GR::u32 dwType, const tTextureSection& TexSection );

    static GUIComponentDisplayer&  Instance();

    friend class GUIComponent;



};



#endif // GUI_COMPONENT_DISPLAYER_H



