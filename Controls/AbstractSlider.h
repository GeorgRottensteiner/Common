#ifndef ABSTRACTSLIDER_H
#define ABSTRACTSLIDER_H



#include <string>
#include <vector>

#include <String/XML.h>

#include <String/Convert.h>

#include <Interface/IService.h>



template <class BASECLASS> class AbstractSlider : public BASECLASS
{

  public:

    enum SliderFlagType
    {
      SFT_DEFAULT       = 0x00000000,
      SFT_DRAGGING      = 0x00000001,
      SFT_VERTICAL      = SFT_DEFAULT,
      SFT_HORIZONTAL    = 0x80000000,
    };


  protected:

    int                   m_SliderLength,
                          m_FullLength,
                          m_SliderOffset,
                          m_SliderValue,

                          m_SliderHitOffset,
                          m_MouseWheelFactor,

                          m_PageSteps;     // Schrittweite, wenn neben den Slider geklickt


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
    using BASECLASS::SetColor;
    using BASECLASS::GetSysColor;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::ModifyStyle;



    AbstractSlider( GR::u32 Id = 0 ) :
        BASECLASS( Id )
    {
      m_ClassName         = "Slider";

      m_Style             = SFT_DEFAULT;
      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;
      m_SliderLength     = 100;
      m_FullLength       = 100;
      m_SliderOffset     = 0;
      m_SliderValue      = 0;
      m_SliderHitOffset  = 0;
      m_PageSteps        = 10;
      m_MouseWheelFactor  = 3;

      BASECLASS::SetBaseColors();
      SetSizes( m_FullLength );
    }



    AbstractSlider( int X, int Y, int Width, int Height, GR::u32 Flags, GR::u32 Id = 0 ) :
      BASECLASS( Id )
    {
      m_ClassName         = "Slider";

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_Position.x        = X;
      m_Position.y        = Y;

      m_Width             = Width;
      m_Height            = Height;
      if ( m_Width < 0 )
      {
        m_Width = 0;
      }
      if ( m_Height < 0 )
      {
        m_Height = 0;
      }

      m_Style            = Flags;

      m_SliderLength     = m_Height;
      m_FullLength       = m_Height;
      m_PageSteps        = m_Height / 10;
      m_MouseWheelFactor  = 3;
      if ( Style() & AbstractSlider::SFT_HORIZONTAL )
      {
        m_PageSteps = m_Width / 10;
      }
      if ( m_PageSteps == 0 )
      {
        m_PageSteps = 1;
      }
      if ( m_SliderLength == 0 )
      {
        m_SliderLength = 20;
      }
      m_SliderOffset     = 0;
      m_SliderValue      = 0;
      m_SliderHitOffset  = 0;

      RecalcClientRect();
      SetSizes( m_FullLength );
      BASECLASS::SetBaseColors();
    }



    virtual void GetSliderRect( GR::tRect& rc )
    {
      if ( m_FullLength == 0 )
      {
        rc.Position( 0, 0 );
        rc.Size( BASECLASS::m_ClientRect.Width(), BASECLASS::m_ClientRect.Height() );
        return;
      }
      if ( Style() & SFT_HORIZONTAL )
      {
        rc.Position( m_SliderOffset, 0 );
        rc.Size( m_SliderLength, BASECLASS::m_ClientRect.Height() );
      }
      else
      {
        rc.Position( 0, m_SliderOffset );
        rc.Size( BASECLASS::m_ClientRect.Width(), m_SliderLength );
      }
    }



    virtual void SetSliderPosition( int Pos )
    {
      if ( m_FullLength < 0 )
      {
        if ( Pos > 0 )
        {
          Pos = 0;
        }
        if ( Pos < m_FullLength )
        {
          Pos = m_FullLength;
        }
      }
      else
      {
        if ( Pos < 0 )
        {
          Pos = 0;
        }
        if ( Pos > m_FullLength )
        {
          Pos = m_FullLength;
        }
      }
      int   iOldValue = m_SliderValue;
      m_SliderValue = Pos;

      if ( m_FullLength == 0 )
      {
        m_SliderOffset = 0;
      }
      else
      {
        if ( Style() & SFT_HORIZONTAL )
        {
          m_SliderOffset = m_SliderValue * ( m_Width - m_SliderLength ) / m_FullLength;
        }
        else
        {
          m_SliderOffset = m_SliderValue * ( m_Height - m_SliderLength ) / m_FullLength;
        }
      }
      if ( m_SliderValue != iOldValue )
      {
        GenerateEventForParent( OET_SLIDER_SCROLLED, m_SliderValue );
        BASECLASS::Invalidate();
      }
    }



    virtual void SetSliderOffset( int Pos )
    {
      if ( Pos < 0 )
      {
        Pos = 0;
      }
      int   Value = 0;
      if ( Style() & SFT_HORIZONTAL )
      {
        if ( Pos > m_Width - m_SliderLength )
        {
          Pos = m_Width - m_SliderLength;
        }
        if ( m_Width - m_SliderLength == 0 )
        {
          Value = 0;
        }
        else
        {
          Value = Pos * m_FullLength / ( m_Width - m_SliderLength );
        }
      }
      else
      {
        if ( Pos > m_Height - m_SliderLength )
        {
          Pos = m_Height - m_SliderLength;
          if ( Pos < 0 )
          {
            Pos = 0;
          }
        }
        if ( m_Height - m_SliderLength == 0 )
        {
          Value = 0;
        }
        else
        {
          if ( m_Height - m_SliderLength == 0 )
          {
            Value = 0;
          }
          else
          {
            Value = Pos * m_FullLength / ( m_Height - m_SliderLength );
          }
        }
      }
      m_SliderOffset = Pos;
      if ( m_SliderValue != Value )
      {
        m_SliderValue = Value;
        GenerateEventForParent( OET_SLIDER_SCROLLED, m_SliderValue );
        BASECLASS::Invalidate();
      }
    }



    virtual void ScrollDelta( int iDelta )
    {
      SetSliderPosition( m_SliderValue + iDelta );
    }



    virtual int GetSliderOffset()
    {
      return m_SliderValue;
    }



    virtual int GetSliderLength()
    {
      return m_SliderLength;
    }



    virtual int MouseWheelFactor() const
    {
      return m_MouseWheelFactor;
    }

    
    
    virtual void MouseWheelFactor( const int WheelFactor )
    {
      m_MouseWheelFactor = WheelFactor;
    }



    virtual bool IsSliderResizeable()
    {
      return true;
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( ( !IsVisible() )
      ||   ( !IsEnabled() ) )
      {
        return true;
      }

      switch ( Event.Type )
      {
        case CET_MOUSE_WHEEL:
          if ( IsMouseInside() )
          {
            ScrollDelta( (int)Event.Value * m_MouseWheelFactor );
          }
          break;
        case CET_MOUSE_DOWN:
          if ( IsMouseInside() )
          {
            if ( !( Style() & SFT_DRAGGING ) )
            {
              GR::tRect   rc;

              GetSliderRect( rc );
              if ( rc.Contains( Event.Position ) )
              {
                SetCapture();
                ModifyStyle( SFT_DRAGGING );
                if ( m_FullLength == 0 )
                {
                  m_SliderHitOffset = 0;
                }
                else
                {
                  if ( Style() & SFT_HORIZONTAL )
                  {
                    m_SliderHitOffset = Event.Position.x - m_SliderOffset;
                  }
                  else
                  {
                    m_SliderHitOffset = Event.Position.y - m_SliderOffset;
                  }
                }
              }
              else if ( Style() & SFT_HORIZONTAL )
              {
                if ( Event.Position.x < rc.Left )
                {
                  // links davon
                  ScrollDelta( -m_PageSteps );
                }
                else
                {
                  // rechts davon
                  ScrollDelta( m_PageSteps );
                }
              }
              else
              {
                if ( Event.Position.y < rc.Top )
                {
                  // links davon
                  ScrollDelta( -m_PageSteps );
                }
                else
                {
                  // rechts davon
                  ScrollDelta( m_PageSteps );
                }
              }
            }
          }
          break;
        case CET_MOUSE_UP:
          if ( Style() & SFT_DRAGGING )
          {
            GenerateEvent( OET_SLIDER_END_DRAG );
            ModifyStyle( 0, SFT_DRAGGING );
            ReleaseCapture();
          }
          break;
        case CET_MOUSE_UPDATE:
          if ( Event.MouseButtons & 1 )
          {
            if ( Style() & SFT_DRAGGING )
            {
              if ( Style() & SFT_HORIZONTAL )
              {
                SetSliderOffset( Event.Position.x - m_SliderHitOffset );
              }
              else
              {
                SetSliderOffset( Event.Position.y - m_SliderHitOffset );
              }
            }
          }
          break;
        case CET_SET_SIZE:
          BASECLASS::ProcessEvent( Event );
          if ( Style() & SFT_HORIZONTAL )
          {
            SetSizes( BASECLASS::Width(), m_SliderLength );
          }
          else
          {
            SetSizes( BASECLASS::Height(), m_SliderLength );
          }
          return true;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    virtual void SetSizes( int FullLength, int SliderLength = 0 )
    {
      m_FullLength     = FullLength;
      if ( SliderLength == 0 )
      {
        if ( m_FullLength )
        {
          m_SliderLength = 20;
        }
        else
        {
          m_SliderLength = 0;
        }
      }
      else
      {
        m_SliderLength = SliderLength;
      }
      int   Offset = m_SliderHitOffset;
      if ( m_FullLength == 0 )
      {
        Offset = 0;
      }
      else
      {
        if ( Style() & SFT_HORIZONTAL )
        {
          Offset = m_SliderValue * ( BASECLASS::m_ClientRect.Width() - m_SliderLength ) / m_FullLength;
        }
        else
        {
          Offset = m_SliderValue * ( BASECLASS::m_ClientRect.Height() - m_SliderLength ) / m_FullLength;
        }
      }
      SetSliderOffset( Offset );
    }



    virtual GR::i32 FullLength() const
    {
      return m_FullLength;
    }



    virtual void PropertiesToXML( GR::Strings::XMLElement* pElement )
    {
      BASECLASS::PropertiesToXML( pElement );

      pElement->AddAttribute( "FullLength", Misc::Format() << m_FullLength );
      pElement->AddAttribute( "SliderLength", Misc::Format() << m_SliderLength );
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BASECLASS::ParseXML( pElement, Environment );

      int   fullLength = m_FullLength;
      if ( pElement->HasAttribute( "FullLength" ) )
      {
        fullLength = GR::Convert::ToI32( pElement->Attribute( "FullLength" ) );
      }
      int   sliderLength = m_SliderLength;
      if ( pElement->HasAttribute( "SliderLength" ) )
      {
        sliderLength = GR::Convert::ToI32( pElement->Attribute( "SliderLength" ) );
      }

      SetSizes( fullLength, sliderLength );
    }



    void SetPageSteps( int PageSize )
    {
      m_PageSteps = PageSize;
    }


};


#endif // __ABSTRACTSLIDER_H__



