#ifndef ABSTRACTSCROLLBAR_H
#define ABSTRACTSCROLLBAR_H



#include <string>
#include <vector>

#include <Controls\AbstractButton.h>
#include <Controls\AbstractSlider.h>

#include <Controls\OutputEvent.h>
#include <Controls\Component.h>

#include <Misc/Format.h>



template <class BS_, class BT_, class SC_> class AbstractScrollbar : public BS_
{

  public:

    typedef BS_   BASECLASS;
    typedef BT_   BUTTONCLASS;
    typedef SC_   SLIDERCLASS;
  
    enum ScrollbarFlagType
    {
      SBFT_DEFAULT        = 0x00000000l,
      SBFT_HORIZONTAL     = 0x80000000,
      SBFT_VERTICAL       = 0x00000000,

      SB_BUTTON_LEFT_UP   = 0xfffffff0,
      SB_SLIDER           = 0xfffffff1,
      SB_BUTTON_RIGHT_DOWN= 0xfffffff2,
    };


  protected:

    BUTTONCLASS*          m_pButtonLeftUp;
    BUTTONCLASS*          m_pButtonRightDown;
    SLIDERCLASS*          m_pSlider;

    int                   m_Min,
                          m_Max,
                          m_Value,
                          m_MouseWheelFactor,
                          m_VisibleArea;


  public:


    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::Add;



    AbstractScrollbar( GR::u32 dwId = 0 ) :
        BASECLASS( dwId )
    {
      m_ClassName         = "Scrollbar";

      m_Min               = 0;
      m_Max               = 100;
      m_Value             = 0;
      m_VisibleArea       = 0;
      m_MouseWheelFactor  = 3;

      m_Style             = SBFT_DEFAULT;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_pButtonLeftUp     = new BUTTONCLASS( SB_BUTTON_LEFT_UP );
      m_pButtonLeftUp->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pButtonLeftUp->ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
      m_pButtonRightDown  = new BUTTONCLASS( SB_BUTTON_RIGHT_DOWN );
      m_pButtonRightDown->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pButtonRightDown->ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
      m_pSlider           = new SLIDERCLASS( SB_SLIDER );
      m_pSlider->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pSlider->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );

      m_pButtonLeftUp->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pButtonRightDown->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pSlider->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pButtonLeftUp );
      Add( m_pButtonRightDown );
      Add( m_pSlider );

      m_pButtonLeftUp->AddEventListener( this );
      m_pButtonRightDown->AddEventListener( this );
    }



    AbstractScrollbar( int X, int Y, int Width, int Height, GR::u32 Type, GR::u32 Id = 0 ) :
      BASECLASS( Id )  
    {
      m_ClassName         = "Scrollbar";

      m_Position.x        = X;
      m_Position.y        = Y;
      m_MouseWheelFactor  = 3;

      m_Width             = Width;
      m_Height            = Height;

      m_Min               = 0;
      m_Max               = 100;
      m_Value             = 0;
      m_VisibleArea       = 0;

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      RecalcClientRect();

      int   buttonWidth = 20;
      
      if ( Width < buttonWidth )
      {
        buttonWidth = Width;
      }
      if ( Height < buttonWidth )
      {
        buttonWidth = Height;
      }

      if ( IsHorizontal() )
      {
        m_pButtonLeftUp     = new BUTTONCLASS( 0, 0, buttonWidth, Height, SB_BUTTON_LEFT_UP );
        m_pButtonRightDown  = new BUTTONCLASS( Width - buttonWidth, 0, buttonWidth, Height, SB_BUTTON_RIGHT_DOWN );
        m_pSlider           = new SLIDERCLASS( buttonWidth, 0, Width - 2 * buttonWidth, Height, SLIDERCLASS::SFT_HORIZONTAL, SB_SLIDER );
      }
      else
      {
        m_pButtonLeftUp     = new BUTTONCLASS( 0, 0, Width, buttonWidth, SB_BUTTON_LEFT_UP );
        m_pButtonRightDown  = new BUTTONCLASS( 0, Width - buttonWidth, Height, buttonWidth, SB_BUTTON_RIGHT_DOWN );
        m_pSlider           = new SLIDERCLASS( 0, buttonWidth, Width, Height - 2 * buttonWidth, SLIDERCLASS::SFT_VERTICAL, SB_SLIDER );
      }

      m_pButtonLeftUp->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pButtonLeftUp->ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
      m_pButtonRightDown->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pButtonRightDown->ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
      m_pSlider->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pButtonLeftUp->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pButtonRightDown->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pSlider->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      m_pSlider->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );

      Add( m_pButtonLeftUp );
      Add( m_pButtonRightDown );
      Add( m_pSlider );

      m_pButtonLeftUp->AddListener( this );
      m_pButtonRightDown->AddListener( this );
      m_pSlider->AddListener( this );

      m_pSlider->SetSizes( m_Max - m_Min );
    }



    bool IsHorizontal()
    {
      return !!( Style() & SBFT_HORIZONTAL );
    }



    virtual void SetScrollRange( int Min, int Max, int Visible = 0 )
    {
      m_Min  = Min;
      m_Max  = Max;
      if ( m_Value < m_Min )
      {
        m_Value = m_Min;
      }
      if ( m_Value > m_Max )
      {
        m_Value = m_Max;
      }
      if ( Visible == 0 )
      {
        Visible = 20;
      }
      m_VisibleArea = Visible;
      if ( m_Min == m_Max )
      {
        m_pSlider->Enable( false );
        m_pButtonLeftUp->Enable( false );
        m_pButtonRightDown->Enable( false );
      }
      else
      {
        m_pSlider->Enable();
        m_pButtonLeftUp->Enable();
        m_pButtonRightDown->Enable();
      }

      if ( ( ( Visible == 0 )
      &&     ( m_Max - m_Min != 0 ) )
      ||   ( ( Visible != 0 )
      &&     ( Visible + m_Max - m_Min != 0 ) ) )
      {
        if ( m_pSlider->IsSliderResizeable() )
        {
          if ( IsHorizontal() )
          {
            m_pSlider->SetSizes( m_pSlider->Width(), m_pSlider->Width() * Visible / ( Visible + m_Max - m_Min ) );
          }
          else
          {
            m_pSlider->SetSizes( m_pSlider->Height(), m_pSlider->Height() * Visible / ( Visible + m_Max - m_Min ) );
          }
        }
        else
        {
          if ( IsHorizontal() )
          {
            m_pSlider->SetSizes( m_pSlider->Width(), m_pSlider->GetSliderLength() );
          }
          else
          {
            m_pSlider->SetSizes( m_pSlider->Height(), m_pSlider->GetSliderLength() );
          }
        }
      }
      else
      {
        m_pSlider->SetSizes( 0, 0 );
      }
    }



    int GetMin() const
    {
      return m_Min;
    }



    int GetMax() const
    {
      return m_Max;
    }



    void SetPageSteps( int PageSize )
    {
      m_pSlider->SetPageSteps( PageSize );
    }



    virtual void SetScrollPosition( int Pos )
    {
      if ( m_Min == m_Max )
      {
        return;
      }
      if ( m_Min > m_Max )
      {
        if ( Pos < m_Max )
        {
          Pos = m_Max;
        }
        if ( Pos > m_Min )
        {
          Pos = m_Min;
        }
      }
      else
      {
        if ( Pos < m_Min )
        {
          Pos = m_Min;
        }
        if ( Pos > m_Max )
        {
          Pos = m_Max;
        }
      }

      if ( m_Value != Pos )
      {
        m_Value = Pos;
        int   newPos = ( m_Value - m_Min ) * ( m_pSlider->FullLength() - m_pSlider->GetSliderLength() ) / ( m_Max - m_Min + 1 );
        int   realValue = m_Min + ( m_Max - m_Min + 1 ) * newPos / ( m_pSlider->FullLength() - m_pSlider->GetSliderLength() );

        // hack - if at max move slider to the very end
        if ( ( ( m_Min > m_Max )
        &&     ( m_Value == m_Min ) )
        ||   ( ( m_Min <= m_Max )
        &&     ( m_Value == m_Max ) ) )
        {
          newPos    = m_pSlider->FullLength() - m_pSlider->GetSliderLength();
          realValue = m_Value;
        }

        while ( realValue < m_Value )
        {
          ++newPos;
          realValue = m_Min + ( m_Max - m_Min + 1 ) * newPos / ( m_pSlider->FullLength() - m_pSlider->GetSliderLength() );
        }
        while ( realValue > m_Value )
        {
          --newPos;
          realValue = m_Min + ( m_Max - m_Min + 1 ) * newPos / ( m_pSlider->FullLength() - m_pSlider->GetSliderLength() );
        }
        m_pSlider->SetSliderPosition( newPos );

        GenerateEventForParent( OET_SCROLLBAR_SCROLLED, m_Value );
      }
    }



    virtual int GetScrollPosition() const
    {
      return m_Value;
    }
    


    virtual int MouseWheelFactor() const
    {
      return m_MouseWheelFactor;
    }



    virtual void MouseWheelFactor( const int WheelFactor )
    {
      m_MouseWheelFactor = WheelFactor;
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( ( !IsVisible() )
      &&   ( !IsEnabled() ) )
      {
        return true;
      }

      switch ( Event.Type )
      {
        case CET_SET_SIZE:
          if ( IsHorizontal() )
          {
            m_pButtonLeftUp->SetSize( m_pButtonLeftUp->Width(), BASECLASS::Height() );
            m_pButtonRightDown->SetSize( m_pButtonRightDown->Width(), BASECLASS::Height() );
          }
          else
          {
            m_pButtonLeftUp->SetSize( BASECLASS::Width(), m_pButtonLeftUp->Height() );
            m_pButtonRightDown->SetSize( BASECLASS::Width(), m_pButtonRightDown->Height() );
          }
          break;
        case CET_MOUSE_WHEEL:
          if ( m_Min > m_Max )
          {
            ScrollDelta( -(int)Event.Value * m_MouseWheelFactor );
          }
          else if ( m_Min < m_Max )
          {
            ScrollDelta( (int)Event.Value * m_MouseWheelFactor );
          }
          return true;
        case CET_KEY_DOWN:
          if ( !IsHorizontal() )
          {
            if ( ( Event.Character == Xtreme::KEY_DOWN )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
            {
              if ( m_Min > m_Max )
              {
                ScrollDelta( -1 );
              }
              else
              {
                ScrollDelta( 1 );
              }
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_UP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_8 ) )
            {
              if ( m_Min > m_Max )
              {
                ScrollDelta( 1 );
              }
              else
              {
                ScrollDelta( -1 );
              }
              return true;
            }
          }
          else
          {
            if ( ( Event.Character == Xtreme::KEY_RIGHT )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_6 ) )
            {
              if ( m_Min > m_Max )
              {
                ScrollDelta( -1 );
              }
              else
              {
                ScrollDelta( 1 );
              }
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_LEFT )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_4 ) )
            {
              if ( m_Min > m_Max )
              {
                ScrollDelta( 1 );
              }
              else
              {
                ScrollDelta( -1 );
              }
              return true;
            }
          }
          if ( ( Event.Character == Xtreme::KEY_HOME )
          ||   ( Event.Character == Xtreme::KEY_NUMPAD_7 ) )
          {
            if ( m_Min > m_Max )
            {
              SetScrollPosition( m_Max );
            }
            else
            {
              SetScrollPosition( m_Min );
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_END )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_1 ) )
          {
            if ( m_Min > m_Max )
            {
              SetScrollPosition( m_Min );
            }
            else
            {
              SetScrollPosition( m_Max );
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_PAGEUP )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_9 ) )
          {
            if ( m_Min > m_Max )
            {
              ScrollDelta( 10 );
            }
            else
            {
              ScrollDelta( -10 );
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_PAGEDOWN )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_3 ) )
          {
            if ( m_Min > m_Max )
            {
              ScrollDelta( -10 );
            }
            else
            {
              ScrollDelta( 10 );
            }
            return true;
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.Type == OET_BUTTON_PUSHED )
      {
        if ( Event.pComponent == m_pButtonLeftUp )
        {
          if ( m_Min > m_Max )
          {
            ScrollDelta( 1 );
          }
          else
          {
            ScrollDelta( -1 );
          }
        }
        else if ( Event.pComponent == m_pButtonRightDown )
        {
          if ( m_Min > m_Max )
          {
            ScrollDelta( -1 );
          }
          else
          {
            ScrollDelta( 1 );
          }
        }
      }
      else if ( Event.Type == OET_SLIDER_SCROLLED )
      {
        if ( Event.pComponent == m_pSlider )
        {
          if ( m_Max != m_Min )
          {
            int   newPos = m_Min + m_pSlider->GetSliderOffset();

            int   realValue = m_Min + ( m_Max - m_Min + 1 ) * newPos / ( m_pSlider->FullLength() - m_pSlider->GetSliderLength() );
            if ( realValue > m_Max )
            {
              realValue = m_Max;
            }
            else if ( realValue < m_Min )
            {
              realValue = m_Min;
            }
            dh::Log( "Slider result %d (pos %d, full length %d, height %d)", realValue, newPos, m_pSlider->FullLength() - m_pSlider->GetSliderLength(), m_pSlider->Height() );
            if ( realValue != m_Value )
            {
              m_Value = realValue;
              GenerateEventForParent( OET_SCROLLBAR_SCROLLED, realValue );
            }
          }
        }
      }
      return false;
    }



    virtual void SetSize( int Width, int Height )
    {
      GUI::Component::SetSize( Width, Height );

      int   buttonWidth = 20;
      
      if ( Width < buttonWidth )
      {
        buttonWidth = Width;
      }

      if ( Height < buttonWidth )
      {
        buttonWidth = Height;
      }

      if ( IsHorizontal() )
      {
        m_pButtonLeftUp->SetSize( buttonWidth, Height );
        m_pButtonRightDown->SetLocation( Width - buttonWidth, 0 );
        m_pButtonRightDown->SetSize( buttonWidth, Height );

        m_pSlider->SetLocation( buttonWidth, 0 );
        m_pSlider->SetSize( Width - 2 * buttonWidth, Height );
      }
      else
      {
        m_pButtonLeftUp->SetSize( Width, buttonWidth );
        m_pButtonRightDown->SetLocation( 0, Height - buttonWidth );
        m_pButtonRightDown->SetSize( Width, buttonWidth );

        m_pSlider->SetLocation( 0, buttonWidth );
        m_pSlider->SetSize( Width, Height - 2 * buttonWidth );
      }
      if ( m_Max - m_Min != 0 )
      {
        if ( m_pSlider->IsSliderResizeable() )
        {
          m_pSlider->SetSizes( m_Max - m_Min - m_VisibleArea, m_pSlider->Height() * m_VisibleArea / ( m_Max - m_Min ) );
        }
        else
        {
          // no visible area interference
          //m_pSlider->SetSizes( m_Max - m_Min - m_VisibleArea, m_pSlider->GetSliderLength() );
          m_pSlider->SetSizes( m_Max - m_Min, m_pSlider->GetSliderLength() );
        }
      }
    }



    SC_* SliderCtrl()
    {
      return m_pSlider;
    }



    BT_* TopLeftButton()
    {
      return m_pButtonLeftUp;
    }



    BT_* BottomRightButton()
    {
      return m_pButtonRightDown;
    }



    virtual void ScrollDelta( int Delta )
    {
      SetScrollPosition( GetScrollPosition() + Delta );
    }



    virtual GR::i32 VisibleLength()
    {
      return m_VisibleArea;
    }



    virtual void VisibleLength( const GR::i32 VisibleArea )
    {
      m_VisibleArea = VisibleArea;

      if ( m_Max - m_Min != 0 )
      {
        m_pSlider->SetSizes( m_Max - m_Min - m_VisibleArea, m_pSlider->Height() * m_VisibleArea / ( m_Max - m_Min ) );
      }
    }



    virtual void PropertiesToXML( GR::Strings::XMLElement* pElement )
    {
      BS_::PropertiesToXML( pElement );

      pElement->AddAttribute( "Min", Misc::Format() << m_Min );
      pElement->AddAttribute( "Max", Misc::Format() << m_Max );
      pElement->AddAttribute( "VisibleArea", Misc::Format() << m_VisibleArea );
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BS_::ParseXML( pElement, Environment );

      if ( IsHorizontal() )
      {
        m_pSlider->ModifyStyle( SLIDERCLASS::SFT_HORIZONTAL, 0 );
      }
      SetScrollRange( GR::Convert::ToI32( pElement->Attribute( "Min" ) ),
                      GR::Convert::ToI32( pElement->Attribute( "Max" ) ) );
      if ( pElement->HasAttribute( "VisibleArea" ) )
      {
        VisibleLength( GR::Convert::ToI32( pElement->Attribute( "VisibleArea" ) ) );
      }
    }


};


#endif // ABSTRACTSCROLLBAR_H