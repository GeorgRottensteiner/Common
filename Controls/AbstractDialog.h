#ifndef ABSTRACTDIALOG_H
#define ABSTRACTDIALOG_H



#include <string>
#include <vector>

#include <Controls/InputEvent.h>



template <class BASECLASS> class AbstractDialog : public BASECLASS
{

  public:

    enum eDialogStyle
    {
      DS_CAPTION       = 0x00000001L,
      DS_CLIPDRAG      = 0x00000002L,    // Caption kann nicht außerhalb des Screens gedraggt werden
      DS_CLIPDRAG_FULLDIALOG  = 0x00000004L, // Dialog kann komplett nicht außerhalb des Screens gedraggt werden
      DS_DEFAULT       = DS_CAPTION,
    };


  protected:

    int                   m_CaptionHeight;


  public:

    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::m_pFont;
    using BASECLASS::Width;
    using BASECLASS::Height;
    using BASECLASS::m_ClientRect;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::IsFocused;
    using BASECLASS::LocalToScreen;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::GetComponentRect;
    using BASECLASS::ModifyFlags;
    using BASECLASS::Add;
    using BASECLASS::Delete;
    using BASECLASS::GetBorderHeight;
    using BASECLASS::GetTopLevelParent;



    AbstractDialog( GR::u32 Id = 0, GR::u32 Type = DS_DEFAULT ) :
        BASECLASS( Id ),
        m_CaptionHeight( 20 )
    {
      m_Style             = Type;
      m_ClassName         = "Dialog";

      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

      if ( Style() & DS_CAPTION )
      {
        m_ComponentFlags = (GUI::ComponentFlagType)( m_ComponentFlags | GUI::COMPFT_DRAG_ENABLED );
      }
    }



    AbstractDialog( int X, int Y, int Width, int Height, GR::u32 Id = 0, GR::u32 Type = DS_DEFAULT ) :
      BASECLASS( X, Y, Width, Height, Id ),
      m_CaptionHeight( 20 )
    {
      m_Style             = Type;
      m_ClassName         = "Dialog";

      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

      RecalcClientRect();
      if ( Style() & DS_CAPTION )
      {
        m_ComponentFlags = (GUI::ComponentFlagType)( m_ComponentFlags | GUI::COMPFT_DRAG_ENABLED );
      }
    }



    AbstractDialog( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0, GR::u32 Type = DS_DEFAULT ) :
      BASECLASS( X, Y, Width, Height, Caption, Id ),
      m_CaptionHeight( 20 )
    {
      m_Style             = Type;
      m_ClassName         = "Dialog";

      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

      RecalcClientRect();
      if ( Style() & DS_CAPTION )
      {
        m_ComponentFlags |= GUI::COMPFT_DRAG_ENABLED;
      }
    }



    virtual void RecalcClientRect()
    {
      BASECLASS::RecalcClientRect();

      if ( Style() & DS_CAPTION )
      {
        m_ClientRect.offset( 0, m_CaptionHeight );
        m_ClientRect.size( m_ClientRect.width(), m_ClientRect.height() - m_CaptionHeight );
      }
    }




    virtual bool GetCaptionRect( GR::tRect& rcCaption )
    {
      if ( Style() & DS_CAPTION )
      {
        GR::tRect   rcComponent;

        GetComponentRect( rcComponent );
        rcCaption.set( m_ClientRect.Left,
                       GetBorderHeight( GUI::BT_EDGE_TOP ),
                       m_ClientRect.width(),
                       m_CaptionHeight );
        return true;
      }
      rcCaption.clear();
      return false;
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( ( !IsVisible() )
      &&   ( !IsEnabled() ) )
      {
        return false;
      }

      switch ( Event.Type )
      {
        case CET_STYLE_CHANGED:
          if ( Style() & DS_CAPTION )
          {
            m_ComponentFlags |= GUI::COMPFT_DRAG_ENABLED;
          }
          else
          {
            m_ComponentFlags &= ~GUI::COMPFT_DRAG_ENABLED;
          }
          break;
        case CET_SET_POSITION:
          if ( Style() & DS_CLIPDRAG )
          {
            GUI::ComponentContainer*   pTopLevel = GetTopLevelParent();

            if ( ( pTopLevel == this )
            ||   ( pTopLevel == NULL ) )
            {
              return BASECLASS::ProcessEvent( Event );
            }
            GR::tRect     rectTopLevel;

            pTopLevel->GetClientRect( rectTopLevel );

            int   iX = Event.Position.x;
            int   iY = Event.Position.y;

            if ( iX < 0 )
            {
              iX = 0;
            }
            if ( iY < 0 )
            {
              iY = 0;
            }
            if ( iX + m_Width >= rectTopLevel.width() )
            {
              iX = rectTopLevel.width() - m_Width;
            }
            // nur bis zur Caption nach unten rausschieben
            if ( iY + m_ClientRect.Top >= rectTopLevel.height() )
            {
              iY = rectTopLevel.height() - m_ClientRect.Top;
            }

            GUI::ComponentEvent   BaseEvent( Event );

            BaseEvent.Position.x = iX;
            BaseEvent.Position.y = iY;
            BASECLASS::ProcessEvent( BaseEvent );
            return false;
          }
          else if ( Style() & DS_CLIPDRAG_FULLDIALOG )
          {
            GUI::ComponentContainer*   pTopLevel = GetTopLevelParent();

            if ( ( pTopLevel == this )
            ||   ( pTopLevel == NULL ) )
            {
              return BASECLASS::ProcessEvent( Event );
            }
            GR::tRect     rectTopLevel;

            pTopLevel->GetClientRect( rectTopLevel );

            int   iX = Event.Position.x;
            int   iY = Event.Position.y;

            if ( iX < 0 )
            {
              iX = 0;
            }
            if ( iY < 0 )
            {
              iY = 0;
            }
            if ( iX + m_Width >= rectTopLevel.width() )
            {
              iX = rectTopLevel.width() - m_Width;
            }
            // nur bis zur Caption nach unten rausschieben
            if ( iY + m_Height >= rectTopLevel.height() )
            {
              iY = rectTopLevel.height() - m_Height;
            }

            GUI::ComponentEvent   BaseEvent( Event );

            BaseEvent.Position.x = iX;
            BaseEvent.Position.y = iY;
            BASECLASS::ProcessEvent( BaseEvent );
            return false;
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }

};


#endif // __ABSTRACTDIALOG_H__



