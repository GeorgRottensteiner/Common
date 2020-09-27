#include "GUIComponentDisplayer.h"
#include "GUIImage.h"



GUI_IMPLEMENT_CLONEABLE( GUIImage, "Image" )



GUIImage::GUIImage( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwID ) :
  AbstractLabel<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, "", GUI::AF_DEFAULT, dwID )
{
}


void GUIImage::Display( GUIComponentDisplayer* pDisplayer )
{
  pDisplayer->DisplayBackground();
}



void GUIImage::SetImage( const XTextureSection& tsImage )
{
  SetClientSize( tsImage.m_Width, tsImage.m_Height );
  GUIComponent::SetTextureSection( GUI::BT_BACKGROUND, tsImage );
}