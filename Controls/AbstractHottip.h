#ifndef ABSTRACTHOTTIP_H
#define ABSTRACTHOTTIP_H



#include <string>
#include <vector>

#include <Controls/OutputEvent.h>
#include <Controls/Component.h>

#include <GR/GRTypes.h>

#include <String/XML.h>



namespace Interface
{
  struct IFont;
};

template <class BASECLASS> class AbstractHottip : public BASECLASS
{

  public:

    enum eHottipStyle
    {
      HTFT_DEFAULT        = 0x00000000L,
    };



  protected:

    GR::u32                 m_WatchComponentID;

    GUI::Component*         m_pWatchComponent;

    GR::f64                 m_InsideTicks;

    GR::u32                 m_DelayBeforeDisplayTicks;

    bool                    m_MouseInsideWatcher;


  public:

    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::m_Caption;
    using BASECLASS::m_pFont;
    using BASECLASS::m_pParentContainer;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::SetSize;
    using BASECLASS::SetLocation;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::Close;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::GetTopLevelParent;
    using BASECLASS::GetComponentParent;



    AbstractHottip( GR::u32 Id = 0 ) :
      BASECLASS( Id ),
      m_pWatchComponent( NULL ),
      m_InsideTicks( 0.0 ),
      m_MouseInsideWatcher( false ),
      m_DelayBeforeDisplayTicks( 500 ),
      m_WatchComponentID( 0 )
    {
      m_ClassName         = "Hottip";
      m_TextAlignment     = GUI::AF_MULTILINE;

      m_Style = HTFT_DEFAULT;
      SetVisible( false );
    }



    AbstractHottip( int X, int Y, int Width, int Height, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Id ),
      m_pWatchComponent( NULL ),
      m_WatchComponentID( 0 ),
      m_InsideTicks( 0.0 ),
      m_MouseInsideWatcher( false ),
      m_DelayBeforeDisplayTicks( 500 )
    {
      m_ClassName         = "Hottip";
      m_TextAlignment     = GUI::AF_MULTILINE | GUI::AF_VCENTER;

      m_Style = HTFT_DEFAULT;
      RecalcClientRect();
      SetVisible( false );
    }



    AbstractHottip( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Caption, Id ),
      m_pWatchComponent( NULL ),
      m_WatchComponentID( 0 ),
      m_InsideTicks( 0.0 ),
      m_MouseInsideWatcher( false ),
      m_DelayBeforeDisplayTicks( 500 )
    {
      m_ClassName         = "Hottip";
      m_TextAlignment     = GUI::AF_MULTILINE | GUI::AF_VCENTER;

      m_Style = HTFT_DEFAULT;
      RecalcClientRect();
      SetVisible( false );
    }



    virtual ~AbstractHottip()
    {
      Detach();
    }



    virtual bool IsMouseInside( GR::tPoint& )
    {
      return false;
    }



    void Detach()
    {
      if ( m_pWatchComponent )
      {
        m_pWatchComponent->RemoveListener( this );
      }
      m_pWatchComponent = NULL;
    }



    void Attach( GUI::Component* pCompWatch )
    {
      if ( m_pWatchComponent )
      {
        m_pWatchComponent->RemoveListener( this );
      }
      m_pWatchComponent = pCompWatch;
      if ( m_pWatchComponent )
      {
        m_pWatchComponent->AddListener( this );

        SetFont( m_pFont );
      }
    }

    
    
    void Attach( GR::u32 ID )
    {
      m_WatchComponentID = ID;
      Attach( GetTopLevelParent()->GetComponent( m_WatchComponentID ) );
    }

    
    
    // Berechnet die aktuelle Position (kann sich durch mehrfache Verschachtelung ja verändern)
    virtual void RecalculateHottipPosition()
    {
      if ( m_pWatchComponent )
      {
        GR::tPoint    ptPos;

        GUI::ComponentContainer::LocalToScreen( ptPos, m_pWatchComponent );

        GUI::Component* pContainer = GetComponentParent();
        if ( pContainer )
        {
          GUI::ComponentContainer::ScreenToLocal( ptPos, pContainer );
        }

        SetLocation( ptPos.x + ( m_pWatchComponent->Width() - m_Width ) / 2, ptPos.y - m_Height - 2 );
      }

      GR::tRect   rectFull;
  
      GetTopLevelParent()->GetClientRect( rectFull );

      if ( m_Position.x < 10 )
      {
        m_Position.x = 10;
      }
      if ( m_Position.y < 10 )
      {
        m_Position.y = 10;
      }
      if ( m_Position.x + m_Width > rectFull.size().x - 10 )
      {
        m_Position.x = rectFull.size().x - 10 - m_Width;
      }
      if ( m_Position.y + m_Height > rectFull.size().y - 20 - 10 )
      {
        m_Position.y = rectFull.size().y - 20 - m_Height;
      }
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.pComponent == m_pWatchComponent )
      {
        if ( Event.Type == OET_MOUSE_IN )
        {
          m_MouseInsideWatcher = true;
          m_InsideTicks = 0.0;
        }
        else if ( Event.Type == OET_DESTROY )
        {
          SetVisible( false );
          m_MouseInsideWatcher = false;
          Detach();
          Close();
        }
        else if ( Event.Type == OET_MOUSE_OUT )
        {
          SetVisible( false );
          m_MouseInsideWatcher = false;
        }
        else if ( Event.Type == OET_SET_POSITION )
        {
          // die Position meines Watchers hat sich verändert!
          RecalculateHottipPosition();
        }
      }
      return false;
    }



    virtual void Update( float ElapsedTime )
    {

      if ( !IsEnabled() )
      {
        if ( IsVisible() )
        {
          SetVisible( false );
        }
        return;
      }
      if ( m_pWatchComponent )
      {
        if ( ( !m_pWatchComponent->IsVisible() )
        ||   ( !m_pWatchComponent->IsEnabled() ) )
        {
          if ( m_MouseInsideWatcher )
          {
            SetVisible( false );
            m_MouseInsideWatcher = false;          
          }
        }
      }

      if ( m_MouseInsideWatcher )
      {
        if ( !IsVisible() )
        {
          m_InsideTicks += 1000.0 * ElapsedTime;
          if ( m_InsideTicks >= m_DelayBeforeDisplayTicks )
          {
            // set visible now
            // make sure our parent is the topmost container

            GUI::ComponentContainer*    pTopLevelParent = GetTopLevelParent();
            if ( ( pTopLevelParent != m_pParentContainer )
            &&   ( pTopLevelParent != this ) )
            {
              if ( m_pParentContainer )
              {
                m_pParentContainer->Remove( this );
              }
              if ( pTopLevelParent )
              {
                pTopLevelParent->Add( this );
              }
            }
            SetVisible();
          }
        }
        if ( IsVisible() )
        {
          RecalculateHottipPosition();
        }
      }
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( Event.Type == CET_ADDED )
      {
        if ( m_WatchComponentID )
        {
          Attach( GetTopLevelParent()->GetComponent( m_WatchComponentID ) );
        }
      }
      else if ( Event.Type == CET_REMOVED )
      {
        if ( ( m_WatchComponentID )
        &&   ( m_pWatchComponent ) )
        {
          m_pWatchComponent->RemoveListener( this );
          m_pWatchComponent = NULL;
        }
      }
      else if ( Event.Type == CET_SET_CAPTION )
      {
        bool result = BASECLASS::ProcessEvent( Event );

        RecalcSize();
        RecalculateHottipPosition();

        return result;
      }
      else if ( Event.Type == CET_ATTACH_COMPONENT )
      {
        Attach( Event.pComponent );
        return true;
      }
      else if ( Event.Type == CET_DETACH_COMPONENT )
      {
        Detach();
        return true;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BASECLASS::ParseXML( pElement, Environment );

      m_WatchComponentID = atoi( pElement->Attribute( "AttachTo" ).c_str() );
    }



    virtual bool IsMouseInsideNonClientArea( const GR::tPoint& )
    {
      return false;
    }



    virtual void SetFont( Interface::IFont* pFont )
    {
      BASECLASS::SetFont( pFont );

      if ( m_pFont )
      {
        RecalcSize();
        RecalculateHottipPosition();
      }
    }



    virtual void RecalcSize()
    {
      if ( m_pFont == NULL )
      {
        return;
      }
      if ( m_TextAlignment == GUI::AF_DEFAULT )
      {
        SetSize( m_pFont->TextLength( m_Caption.c_str() ) + 4, m_pFont->TextHeight( m_Caption.c_str() ) + 4 );
      }
      else if ( m_TextAlignment & GUI::AF_MULTILINE )
      {
        size_t        pos = 0;
        size_t        lineStartPos = 0;
        GR::String   curLine = "";
        int           width = 0;
        int           height = 2;

        while ( pos < m_Caption.length() )
        {
          if ( ( m_Caption[pos] == '\\' )
          &&   ( pos + 1 < m_Caption.length() )
          &&   ( m_Caption[pos + 1] == 'n' ) )
          {
            // ein Zeilenumbruch!
            int   curWidth = m_pFont->TextLength( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 4;
            if ( curWidth > width )
            {
              width = curWidth;
            }
            height += m_pFont->TextHeight( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 2;
            lineStartPos = pos + 2;
            ++pos;
          }
          else if ( m_Caption[pos] == '\n' )
          {
            // ein Zeilenumbruch!
            int   curWidth = m_pFont->TextLength( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 4;
            if ( curWidth > width )
            {
              width = curWidth;
            }
            height += m_pFont->TextHeight( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 2;
            lineStartPos = pos + 1;
          }
          ++pos;
        }
        if ( lineStartPos < pos )
        {
          int   curWidth = m_pFont->TextLength( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 4;
          if ( curWidth > width )
          {
            width = curWidth;
          }
          height += m_pFont->TextHeight( m_Caption.substr( lineStartPos, pos - lineStartPos ) ) + 2;
        }
        SetSize( width, height );
      }
    }



    virtual void TextAlignment( GR::u32 Alignment )
    {
      m_TextAlignment = Alignment;

      RecalcSize();
      RecalculateHottipPosition();
    }

};


#endif // ABSTRACTLABEL_H



