#ifndef ABSTRACTPULLIN_H
#define ABSTRACTPULLIN_H



#include <string>
#include <vector>

#include <Controls/InputEvent.h>
#include <Lang/Service.h>
#include <String/XML.h>



template <class BASECLASS> class AbstractPullIn : public BASECLASS
{

  public:

    enum ePullInStyle
    {
      PCS_BOTTOM        = 0x00000000,
      PCS_TOP           = 0x00000001,
      PCS_LEFT          = 0x00000002,
      PCS_RIGHT         = 0x00000003,
      PCS_SIDE_MASK     = 0x00000003,
      PCS_ALWAYS_OUT    = 0x00000004,
      PCS_DEFAULT       = PCS_BOTTOM,
    };


  protected:

    int                   m_CaptionSize;

    float                 m_SlideInSpeed,
                          m_SlideOutSpeed,
                          m_SlideDelta;

    GR::tFPoint           m_SlidePos;


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
    using BASECLASS::IsIndirectChild;
    using BASECLASS::LocalToScreen;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::Add;
    using BASECLASS::Delete;
    using BASECLASS::GetSysColor;
    using BASECLASS::SetColor;
    using BASECLASS::GetTopLevelParent;
    using BASECLASS::SetLocation;
    using BASECLASS::GetParentContainer;
    using BASECLASS::GetBorderWidth;
    using BASECLASS::GetBorderHeight;



    AbstractPullIn( GR::u32 Id = 0, GR::u32 Type = PCS_DEFAULT ) :
      BASECLASS( Id ),
      m_CaptionSize( 20 ),
      m_SlideInSpeed( 200.0f ),
      m_SlideOutSpeed( 800.0f )
    {
      m_Style             = Type;
      m_ClassName         = "PullIn";
      m_SlideDelta        = 0.0f;

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    AbstractPullIn( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id = 0, GR::u32 Type = PCS_DEFAULT ) :
        BASECLASS( NewX, NewY, NewWidth, NewHeight, Id ),
        m_CaptionSize( 20 ),
        m_SlideInSpeed( 200.0f ),
        m_SlideOutSpeed( 800.0f )
    {
      m_Style             = Type;
      m_ClassName         = "PullIn";
      m_SlideDelta        = 0.0f;

      RecalcClientRect();

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    AbstractPullIn( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id = 0, GR::u32 Type = PCS_DEFAULT ) :
        BASECLASS( NewX, NewY, NewWidth, NewHeight, Caption, Id ),
        m_CaptionSize( 20 ),
        m_SlideInSpeed( 200.0f ),
        m_SlideOutSpeed( 800.0f )
    {
      m_Style             = Type;
      m_ClassName         = "PullIn";
      m_SlideDelta        = 0.0f;

      RecalcClientRect();

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    void SetCaptionSize( const GR::u32 CaptionSize )
    {
      if ( CaptionSize )
      {
        m_CaptionSize = CaptionSize;
        RecalcClientRect();
      }
    }



    void SlideIn()
    {
      GUI::ComponentContainer*    pTopParent = GetTopLevelParent();

      if ( pTopParent == NULL )
      {
        return;
      }

      GR::tPoint    ptPos;

      GetParentContainer()->LocalToScreen( ptPos, this );

      GR::tRect   rcTopParent;

      pTopParent->GetClientRect( rcTopParent );

      m_SlideDelta = 0.0f;

      if ( ( Style() & PCS_SIDE_MASK ) == PCS_BOTTOM )
      {
        SetLocation( ptPos.x, rcTopParent.Height() - m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_TOP )
      {
        SetLocation( ptPos.x, m_CaptionSize - Height() );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_LEFT )
      {
        SetLocation( m_CaptionSize - Width(), ptPos.y );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_RIGHT )
      {
        SetLocation( rcTopParent.Width() - m_CaptionSize, ptPos.y );
      }
    }



    void SlideOut()
    {
      GUI::ComponentContainer*    pTopParent = GetTopLevelParent();

      if ( pTopParent == NULL )
      {
        return;
      }

      GR::tPoint    ptPos;

      GetParentContainer()->LocalToScreen( ptPos, this );

      GR::tRect   rcTopParent;

      pTopParent->GetClientRect( rcTopParent );

      m_SlideDelta = 0.0f;

      if ( ( Style() & PCS_SIDE_MASK ) == PCS_BOTTOM )
      {
        SetLocation( ptPos.x, rcTopParent.Height() - Height() + m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_TOP )
      {
        SetLocation( ptPos.x, 0 );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_LEFT )
      {
        SetLocation( 0, ptPos.y );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_RIGHT )
      {
        SetLocation( rcTopParent.Width() - Width() + m_CaptionSize, ptPos.y );
      }
    }

    virtual void          RecalcClientRect()
    {
      BASECLASS::RecalcClientRect();

      if ( ( Style() & PCS_SIDE_MASK ) == PCS_BOTTOM )
      {
        m_ClientRect.Offset( 0, m_CaptionSize );
        m_ClientRect.Size( m_ClientRect.Width(), m_ClientRect.Height() - m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_TOP )
      {
        m_ClientRect.Offset( 0, 0 );
        m_ClientRect.Size( m_ClientRect.Width(), m_ClientRect.Height() - m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_RIGHT )
      {
        m_ClientRect.Offset( m_CaptionSize, 0 );
        m_ClientRect.Size( m_ClientRect.Width() - m_CaptionSize, m_ClientRect.Height() );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_LEFT )
      {
        m_ClientRect.Offset( 0, 0 );
        m_ClientRect.Size( m_ClientRect.Width() - m_CaptionSize, m_ClientRect.Height() );
      }
    }

    bool GetCaptionRect( GR::tRect& rcCaption )
    {
      if ( ( Style() & PCS_SIDE_MASK ) == PCS_BOTTOM )
      {
        rcCaption.Set( m_ClientRect.Left,
                       GetBorderHeight( GUI::BT_EDGE_TOP ),
                       m_ClientRect.Width(),
                       m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_TOP )
      {
        rcCaption.Set( m_ClientRect.Left,
                       m_ClientRect.Bottom,
                       m_ClientRect.Width(),
                       m_CaptionSize );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_RIGHT )
      {
        rcCaption.Set( GetBorderWidth( GUI::BT_EDGE_LEFT ),
                        m_ClientRect.Top,
                        m_CaptionSize,
                        m_ClientRect.Height() );
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_LEFT )
      {
        rcCaption.Set( m_ClientRect.Right,
                        m_ClientRect.Top,
                        m_CaptionSize,
                        m_ClientRect.Height() );
      }
      return true;
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
        case CET_ADDED:
          if ( Style() & PCS_ALWAYS_OUT )
          {
            SlideOut();
          }
          else
          {
            SlideIn();
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    bool IsMouseOverMe()
    {
      if ( IsMouseInside() )
      {
        return true;
      }
      GUI::ComponentContainer*    pTopParent = GetTopLevelParent();

      if ( ( pTopParent )
      &&   ( pTopParent->MouseOverComponent() ) )
      {
        if ( pTopParent->MouseOverComponent()->IsIndirectChild( this ) )
        {
          return true;
        }

        // is the owner of the current mouse over component a indirect child of us? (for open lists of combos!)
        GUI::Component*    pComponent = pTopParent->MouseOverComponent();
        if ( IsIndirectChild( pComponent->Owner() ) )
        {
          return true;
        }
      }
      return false;
    }



    virtual void Update( float fElapsedTime )
    {
      GUI::ComponentContainer*    pTopParent = GetTopLevelParent();

      if ( pTopParent == NULL )
      {
        return;
      }
      if ( pTopParent->ModalDisabled( this ) )
      {
        return;
      }

      if ( Style() & PCS_ALWAYS_OUT )
      {
        return;
      }

      GR::tPoint    ptPos;

      GetParentContainer()->LocalToScreen( ptPos, this );

      GR::tRect   rcTopParent;

      pTopParent->GetClientRect( rcTopParent );

      // mouse outside window etc.
      if ( ( !pTopParent->IsMouseOverComponent() )
      &&   ( !IsMouseOverMe() ) )
      {
        return;
      }
      /*
      TODO - remove that
      Xtreme::XInput* pInput = (Xtreme::XInput*)GR::Service::Environment::Instance().Service( "Input" );
      if ( pInput )
      {
        if ( !rcTopParent.contains( pInput->MousePos() ) )
        {
          bOutSideParent = true;
          return;
        }
      }*/

      if ( ( Style() & PCS_SIDE_MASK ) == PCS_BOTTOM )
      {
        if ( IsMouseOverMe() )
        {
          if ( ptPos.y > rcTopParent.Height() - Height() + m_CaptionSize )
          {
            if ( fElapsedTime * m_SlideOutSpeed >= Height() - m_CaptionSize - m_SlideDelta )
            {
              SetLocation( ptPos.x, rcTopParent.Height() - Height() + m_CaptionSize );
              m_SlideDelta = (float)( Height() - m_CaptionSize );
            }
            else
            {
              m_SlideDelta += fElapsedTime * m_SlideOutSpeed;
              SetLocation( ptPos.x, rcTopParent.Height() - m_CaptionSize - (int)m_SlideDelta );
            }
          }
        }
        else
        {
          if ( ptPos.y < rcTopParent.Height() - m_CaptionSize )
          {
            if ( fElapsedTime * m_SlideInSpeed >= m_SlideDelta )
            {
              SetLocation( ptPos.x, rcTopParent.Height() - m_CaptionSize );
              m_SlideDelta = 0.0f;
            }
            else
            {
              m_SlideDelta -= fElapsedTime * m_SlideInSpeed;
              SetLocation( ptPos.x, rcTopParent.Height() - m_CaptionSize - (int)m_SlideDelta );
            }
          }
        }
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_TOP )
      {
        if ( IsMouseOverMe() )
        {
          if ( ptPos.y < 0 )
          {
            if ( fElapsedTime * m_SlideOutSpeed >= Height() - m_CaptionSize - m_SlideDelta )
            {
              SetLocation( ptPos.x, 0 );
              m_SlideDelta = (float)( Height() - m_CaptionSize );
            }
            else
            {
              m_SlideDelta += fElapsedTime * m_SlideOutSpeed;
              SetLocation( ptPos.x, m_CaptionSize - Height() + (int)m_SlideDelta );
            }
          }
        }
        else
        {
          if ( ptPos.y > m_CaptionSize - Height() )
          {
            if ( fElapsedTime * m_SlideInSpeed >= m_SlideDelta )
            {
              SetLocation( ptPos.x, Height() - m_CaptionSize );
              m_SlideDelta = 0.0f;
            }
            else
            {
              m_SlideDelta -= fElapsedTime * m_SlideInSpeed;
              SetLocation( ptPos.x, m_CaptionSize - Height() + (int)m_SlideDelta );
            }
          }
        }
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_LEFT )
      {
        if ( IsMouseOverMe() )
        {
          if ( ptPos.x < 0 )
          {
            m_SlidePos.x += fElapsedTime * m_SlideOutSpeed;
            if ( m_SlidePos.x > 0 )
            {
              m_SlidePos.x = 0;
            }
            SetLocation( (int)m_SlidePos.x, ptPos.y );
          }
        }
        else
        {
          if ( ptPos.x > m_CaptionSize - Width() )
          {
            m_SlidePos.x -= fElapsedTime * m_SlideInSpeed;
            if ( m_SlidePos.x < m_CaptionSize - Width() )
            {
              m_SlidePos.x = (float)( m_CaptionSize - Width() );
            }
            SetLocation( (int)m_SlidePos.x, ptPos.y );
          }
        }
      }
      else if ( ( Style() & PCS_SIDE_MASK ) == PCS_RIGHT )
      {
        if ( IsMouseOverMe() )
        {
          if ( ptPos.x > rcTopParent.Width() - Width() )
          {
            m_SlidePos.x -= fElapsedTime * m_SlideOutSpeed;
            if ( m_SlidePos.x < rcTopParent.Width() - Width() )
            {
              m_SlidePos.x = (float)( rcTopParent.Width() - Width() );
            }
            SetLocation( (int)m_SlidePos.x, ptPos.y );
          }
        }
        else
        {
          if ( ptPos.x < rcTopParent.Width() - m_CaptionSize )
          {
            m_SlidePos.x += fElapsedTime * m_SlideInSpeed;
            if ( m_SlidePos.x > rcTopParent.Width() - m_CaptionSize )
            {
              m_SlidePos.x = (float)( rcTopParent.Width() - m_CaptionSize );
            }
            SetLocation( (int)m_SlidePos.x, ptPos.y );
          }
        }
      }
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BASECLASS::ParseXML( pElement, Environment );
      if ( pElement == NULL )
      {
        return;
      }
      if ( pElement->HasAttribute( "CaptionSize" ) )
      {
        m_CaptionSize = GR::Convert::ToI32( pElement->Attribute( "CaptionSize" ) );
      }
    }




};


#endif // ABSTRACTPULLIN_H



