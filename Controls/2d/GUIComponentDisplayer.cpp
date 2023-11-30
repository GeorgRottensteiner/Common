#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"



GUIComponentDisplayer::GUIComponentDisplayer()
{
  GR::Service::Environment::Instance().SetService( "GUI", this );
}



GUIComponentDisplayer::~GUIComponentDisplayer()
{
  GR::Service::Environment::Instance().RemoveService( "GUI" );
}



void GUIComponentDisplayer::SetClipping( int X, int Y, int Width, int Height )
{
  if ( m_pPage == NULL )
  {
    return;
  }
  if ( ( Width <= 0 )
  ||   ( Height <= 0 ) )
  {
    m_NothingIsVisible = true;
  }
  else
  {
    m_NothingIsVisible = false;
  }
  m_pPage->SetRange( X, Y, X + Width - 1, Y + Height - 1 );
}



GR::tPoint GUIComponentDisplayer::GetOffset()
{
  if ( m_pPage == NULL )
  {
    return GR::tPoint();
  }
  return GR::tPoint( m_pPage->GetOffsetX(), m_pPage->GetOffsetY() );
}



void GUIComponentDisplayer::SetOffset( int X, int Y )
{
  if ( m_pPage == NULL )
  {
    return;
  }
  m_pPage->SetOffset( X, Y );
}



void GUIComponentDisplayer::PushClipValues()
{
  // hier das Offset abziehen war neu
  StoreClipValues( m_pPage->GetLeftBorder(), 
                   m_pPage->GetTopBorder(),
                   m_pPage->GetRightBorder() - m_pPage->GetLeftBorder(),// + 1 - m_pPage->GetOffsetX(),
                   m_pPage->GetBottomBorder() - m_pPage->GetTopBorder(),// + 1 - m_pPage->GetOffsetY(),
                   m_pPage->GetOffsetX(),
                   m_pPage->GetOffsetY() );
}



GUIComponentDisplayer& GUIComponentDisplayer::Instance()
{
  static GUIComponentDisplayer   g_Instance;

  return g_Instance;
}



void GUIComponentDisplayer::SetDefaultTextureSection( GUI::eBorderType Type, GR::Graphic::ContextDescriptor& CDSection )
{
  m_DefaultTextureSection[Type] = CDSection;
}



void GUIComponentDisplayer::DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 )
{
  m_pPage->Box( X, Y, Width, Height, Color );
}



void GUIComponentDisplayer::DrawRect( int X, int Y, int Width, int Height, GR::u32 Color )
{
  m_pPage->Rectangle( X, Y, Width, Height, Color );
}



void GUIComponentDisplayer::DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color )
{
  m_pPage->Rectangle( Pos.x, Pos.y, Size.x, Size.y, Color );
}



void GUIComponentDisplayer::DrawTextureSection( int X,
                                                int Y,
                                                const XTextureSection& TexSection,
                                                GR::u32 Color,
                                                int AlternativeWidth,
                                                int AlternativeHeight,
                                                GR::u32 AlternativeFlags )
{
}



void GUIComponentDisplayer::DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color )
{
}



void GUIComponentDisplayer::DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color )
{
}



void GUIComponentDisplayer::DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color )
{
}



void GUIComponentDisplayer::DrawText( Interface::IFont* pFont,
                                      const GR::String& Text,
                                      const GR::tRect& Rect,
                                      GR::u32 TextAlignment,
                                      GR::u32 Color )
{
}



void GUIComponentDisplayer::DrawText( Interface::IFont* pFont,
                                      int XOffset, int YOffset,
                                      const GR::String& Text,
                                      GR::u32 TextAlignment,
                                      GR::u32 Color,
                                      const GR::tRect* pRect )
{
}



void GUIComponentDisplayer::DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge )
{
}



void GUIComponentDisplayer::DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections )
{
}



void GUIComponentDisplayer::DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 )
{
  m_pPage->Line( Pos1.x, Pos1.y, Pos2.x, Pos2.y, Color );
}
