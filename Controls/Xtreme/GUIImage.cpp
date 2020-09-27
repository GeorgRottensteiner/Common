#include "GUIComponentDisplayer.h"
#include "GUIImage.h"



GUI_IMPLEMENT_CLONEABLE( GUIImage, "Image" )



GUIImage::GUIImage( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 ID ) :
  AbstractLabel<GUIComponent>( NewX, NewY, NewWidth, NewHeight, "", GUI::AF_DEFAULT, ID )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUIImage::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( m_Image.m_pTexture )
  {
    GR::tRect  rcClient;
    GR::tPoint ptClientOffset = GetClientOffset();

    GetClientRect( rcClient );

    if ( ( ( m_Image.m_Flags & XTextureSection::TSF_ROTATE_270 ) != 0 )
    &&   ( ( m_Image.m_Flags & XTextureSection::TSF_ROTATE_270 ) != XTextureSection::TSF_ROTATE_180 ) )
    {
      Displayer.DrawTextureSection( 0, 0, m_Image, GetColor( GUI::COL_BTNFACE ), rcClient.height(), rcClient.width(), m_Image.m_Flags );
    }
    else
    {
      Displayer.DrawTextureSection( 0, 0, m_Image, GetColor( GUI::COL_BTNFACE ), rcClient.width(), rcClient.height() );
    }
  }
}



void GUIImage::SetImage( const XTextureSection& tsImage )
{
  if ( ( ( tsImage.m_Flags & XTextureSection::TSF_ROTATE_270 ) != 0 )
  &&   ( ( tsImage.m_Flags & XTextureSection::TSF_ROTATE_270 ) != XTextureSection::TSF_ROTATE_180 ) )
  {
    SetSize( tsImage.m_Height, tsImage.m_Width );
    SetClientSize( tsImage.m_Height, tsImage.m_Width );
  }
  else
  {
    SetSize( tsImage.m_Width, tsImage.m_Height );
    SetClientSize( tsImage.m_Width, tsImage.m_Height );
  }
  m_Image = tsImage;
}



void GUIImage::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
{
  AbstractLabel<GUIComponent>::ParseXML( pElement, Environment );

  if ( pElement == NULL )
  {
    return;
  }
  GR::String   sectionName = pElement->Attribute( "Image" );

  Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)Environment.Service( "AssetLoader" );
  if ( pLoader )
  {
    if ( !sectionName.empty() )
    {
      Xtreme::Asset::XAsset* pSection = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, sectionName.c_str() );
      if ( pSection == NULL )
      {
        dh::Log( "GUIImage::ParseXML Section (%s) not found", sectionName.c_str() );
      }
      else
      {
        XTextureSection   section = *(XTextureSection*)pSection->Handle( "Section" );
        SetImage( section );
      }
    }
  }
}