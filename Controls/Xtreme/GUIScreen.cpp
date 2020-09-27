#include "GUIComponentDisplayer.h"
#include "GUIScreen.h"



GUI_IMPLEMENT_CLONEABLE( GUIScreen, "Screen" )



GUIScreen::GUIScreen( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id ) :
  GUIComponent( NewX, NewY, NewWidth, NewHeight, Id )
{
  m_ClassName = "Screen";
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUIScreen::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
{
  m_Image = pElement->Attribute( "Image" );
  GUIComponent::ParseXML( pElement, Environment );
}



void GUIScreen::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  GUIComponent::DisplayNonClientOnPage( Displayer );

  if ( !m_Image.empty() )
  {
    if ( ( m_pParentContainer != NULL )
    &&   ( ( m_pParentContainer != GetTopLevelParent() )
    ||     ( (GUIScreen*)GetTopLevelParent()->GetFirstChildComponent() != this ) ) )
    {
      // we are not topmost, topmost screens have their background drawn by the componentdisplayer directly
      // TODO - ugly mess, grabbing at all kind of globals
      if ( m_Background.m_Textures.empty() )
      {
        Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)GUIComponentDisplayer::Instance().m_pEnvironment->Service( "AssetLoader" );
        if ( pLoader )
        {
          Xtreme::Asset::XAsset* pAssetTexture = pLoader->Asset( Xtreme::Asset::XA_IMAGE, m_Image.c_str() );
          if ( pAssetTexture == NULL )
          {
            dh::Log( "GUIScreen::DisplayOnPage Image (%s) not found", m_Image.c_str() );
          }
          else
          {
            XTexture* pTexture = (XTexture*)pAssetTexture->Handle( "Texture" );
            if ( m_Background.m_pRenderer == NULL )
            {
              m_Background.Initialize( Displayer.m_pActualRenderer, GUIComponentDisplayer::Instance().m_pEventProducer );
            }
            if ( m_Background.m_Filename != pTexture->m_LoadedFromFile )
            {
              m_Background.Clear();
              m_Background.LoadImage( pTexture->m_LoadedFromFile.c_str() );
              //dh::Log( "reload hugetexture %s", pTexture->m_LoadedFromFile.c_str() );
            }
          }
        }
      }
      m_Background.Render2d( *Displayer.m_pActualRenderer, 0, 0 );
    }
  }
}