#include "ComponentDisplayerBase.h"



namespace Interface
{
  struct IFont;
}


namespace GUI
{
  ComponentDisplayerBase* ComponentDisplayerBase::s_pInstance = NULL;


  void WrapText( Interface::IFont* pFont, const GR::String& Text, GR::tRect& TextArea, std::vector<GR::String>& TextLines, bool CalcRect )
  {
    TextLines.clear();

    if ( pFont == NULL )
    {
      return;
    }

    // jetzt Text schnippeln
    GR::String    dummy = Text;

    GR::String    newLine;

    int           pos = -1;

    GR::tRect     rcResult;

    do
    {
      newLine = "";

      ++pos;
      if ( pos >= (int)dummy.length() )
      {
        TextLines.push_back( dummy );
        if ( CalcRect )
        {
          int   pixelLength = pFont->TextLength( dummy.c_str() );
          if ( pixelLength > rcResult.Width() )
          {
            rcResult.Width( pixelLength );
          }
        }
        break;
      }
      if ( (GR::u8)dummy[pos] < 32 )
      {
        if ( dummy[pos] == 10 )
        {
          // Linebreak
          TextLines.push_back( dummy.substr( 0, pos ) );
          dummy = dummy.substr( pos + 1 );
          pos = -1;
          continue;
        }

        dummy.erase( pos, 1 );
        --pos;
        continue;
      }
      if ( ( dummy[pos] == '\\' )   // erzwungener Umbruch
      &&   ( pos < (int)dummy.length() )
      &&   ( dummy[pos + 1] == 'n' ) )
      {
        if ( CalcRect )
        {
          int   iPixelLength = pFont->TextLength(  dummy.substr( 0, pos ).c_str() );
          if ( iPixelLength > rcResult.Width() )
          {
            rcResult.Width( iPixelLength );
          }
        }
        TextLines.push_back( dummy.substr( 0, pos ) );
        dummy = dummy.substr( pos + 2 );
        pos = -1;
      }
      else if ( pFont->TextLength( dummy.substr( 0, pos ).c_str() ) >= TextArea.Width() )
      {
        if ( dummy[pos] != '\n' )
        {
          // hier muﬂ noch eine passende Stelle (Leerzeichen) gefunden werden
          int   newPos = pos;
          do
          {
            newPos--;
            if ( newPos <= 0 )
            {
              newPos = pos;
              break;
            }
          }
          while ( dummy[newPos] != ' ' );
          pos = newPos;
        }
        if ( CalcRect )
        {
          int   pixelLength = pFont->TextLength(  dummy.substr( 0, pos ).c_str() );
          if ( pixelLength > rcResult.Width() )
          {
            rcResult.Width( pixelLength );
          }
        }
        TextLines.push_back( dummy.substr( 0, pos ) );
        dummy = dummy.substr( pos + 1 );
        pos = -1;
      }
    }
    while ( dummy.length() );

    if ( CalcRect )
    {
      rcResult.Height( (int)TextLines.size() * pFont->TextHeight() );
      TextArea = rcResult;
    }
  }



  void BreakText( Interface::IFont* pFont, const GR::String& strText, GR::tRect& rectResult, std::vector<GR::String>& vectText, bool bCalcRect )
  {
    vectText.clear();

    if ( pFont == NULL )
    {
      return;
    }

    // jetzt Text schnippeln
    GR::String    dummy = strText;
    GR::String    newLine;
    int           pos = -1;
    GR::tRect     rcResult;

    do
    {
      newLine = "";

      ++pos;
      if ( pos >= (int)dummy.length() )
      {
        vectText.push_back( dummy );
        if ( bCalcRect )
        {
          int   pixelLength = pFont->TextLength( dummy.c_str() );
          if ( pixelLength > rcResult.Width() )
          {
            rcResult.Width( pixelLength );
          }
        }
        break;
      }
      if ( (unsigned int)dummy[pos] < 32 )
      {
        if ( dummy[pos] == 10 )
        {
          // Linebreak
          vectText.push_back( dummy.substr( 0, pos ) );
          dummy = dummy.substr( pos + 1 );
          pos = -1;
          continue;
        }

        dummy.erase( pos, 1 );
        --pos;
        continue;
      }
      if ( ( dummy[pos] == '\\' )   // erzwungener Umbruch
      &&   ( pos < (int)dummy.length() )
      &&   ( dummy[pos + 1] == 'n' ) )
      {
        if ( bCalcRect )
        {
          int   pixelLength = pFont->TextLength(  dummy.substr( 0, pos ).c_str() );
          if ( pixelLength > rcResult.Width() )
          {
            rcResult.Width( pixelLength );
          }
        }
        vectText.push_back( dummy.substr( 0, pos ) );
        dummy = dummy.substr( pos + 2 );
        pos = -1;
      }
    }
    while ( dummy.length() );

    if ( bCalcRect )
    {
      rcResult.Height( (int)vectText.size() * pFont->TextHeight() );
      rectResult = rcResult;
    }
  }



  GR::tPoint TextOffset( Interface::IFont* pFont, const GR::String& strText, GR::u32 textAlignment, const GR::tRect& Rect )
  {
    GR::tPoint    ptResult( Rect.Position() );

    if ( pFont == NULL )
    {
      return ptResult;
    }

    if ( ( textAlignment & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
    {
      ptResult.y = Rect.Top + ( Rect.Height() - pFont->TextHeight() ) / 2;
    }
    else if ( textAlignment & GUI::AF_BOTTOM )
    {
      ptResult.y = Rect.Top + Rect.Height() - pFont->TextHeight();
    }

    if ( ( textAlignment & GUI::AF_CENTER ) == GUI::AF_CENTER )
    {
      ptResult.x = Rect.Left + ( Rect.Width() - pFont->TextLength( strText ) ) / 2;
    }
    else if ( textAlignment & GUI::AF_RIGHT )
    {
      ptResult.x = Rect.Left + Rect.Width() - pFont->TextLength( strText );
    }
    return ptResult;
  }



  GR::u64 GetTicks()
  {
#if OPERATING_SYSTEM == OS_ANDROID
    struct  timespec now;
    clock_gettime( CLOCK_MONOTONIC, &now );
    return (GR::u64)( now.tv_sec * 1000 + now.tv_nsec / 1000 );
#else
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>( duration ).count();

    return millis;
#endif
    /*
#if OPERATING_SYSTEM == OS_WINDOWS
#ifdef GetTickCount64
    return GetTickCount64();
#else
    return GetTickCount64();
#endif
#else
    return GetTickCount64();
#endif*/
  }



  GR::u64 GetDoubleClickTimeMS()
  {
#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
    return ::GetDoubleClickTime();
#else
    return 500;
#endif
  }



  void ComponentDisplayerBase::RetrieveDefaultAssets( Xtreme::Asset::XAssetLoader* pLoader )
  {
    if ( pLoader == NULL )
    {
      dh::Log( "CComponentDisplayerBase No AssetLoader found" );
      return;
    }

    Xtreme::Asset::XAssetFont* pFont = (Xtreme::Asset::XAssetFont*)pLoader->Asset( Xtreme::Asset::XA_FONT, "GUI.Default" );
    if ( pFont )
    {
      SetDefaultFont( pFont->Font() );
    }
  }

      
      
  bool ComponentDisplayerBase::OnServiceNotify( const char* Event, GR::IService* pServiceSender )
  {
    if ( strcmp( Event, "AssetLoader.AssetsLoaded" ) == 0 )
    {
      RetrieveDefaultAssets( (Xtreme::Asset::XAssetLoader*)pServiceSender );
      return true;
    }
    return false;
  }



  void ComponentDisplayerBase::OnFrameEvent( const GR::tRenderFrameEvent& Event )
  {
    if ( Event.m_Event == GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED )
    {
      if ( ( Event.m_ptPoint.x == 0 )
      ||   ( Event.m_ptPoint.y == 0 ) )
      {
        return;
      }
      SetExtents( Event.m_ptPoint.x, Event.m_ptPoint.y );
    }
  }



  void ComponentDisplayerBase::OnServiceGotSet( const char* Service, GR::IService* pService )
  {
    if ( strcmp( Service, "WindowFrame" ) == 0 )
    {
      GR::RenderFrame*   pFrame = (GR::RenderFrame*)pService;
      pFrame->AddHandler( fastdelegate::MakeDelegate( this, &ComponentDisplayerBase::OnFrameEvent ) );
    }
    else if ( strcmp( Service, "Input" ) == 0 )
    {
      Xtreme::XInput*     pInput = (Xtreme::XInput*)pService;
      if ( pInput )
      {
        pInput->AddListener( this );
      }
      m_pInputClass = pInput;
    }
  }



  void ComponentDisplayerBase::OnServiceGotUnset( const char* Service, GR::IService* pService )
  {
    if ( strcmp( Service, "WindowFrame" ) == 0 )
    {
      GR::RenderFrame*   pFrame = (GR::RenderFrame*)pService;
      pFrame->RemoveHandler( fastdelegate::MakeDelegate( this, &ComponentDisplayerBase::OnFrameEvent ) );
    }
    else if ( strcmp( Service, "Input" ) == 0 )
    {
      Xtreme::XInput*     pInput = (Xtreme::XInput*)pService;
      if ( pInput )
      {
        pInput->RemoveListener( this );
      }
      m_pInputClass = NULL;
    }
  }



  void ComponentDisplayerBase::OnSet()
  {
    GR::RenderFrame*   pFrame = (GR::RenderFrame*)m_pEnvironment->Service( "WindowFrame" );
    if ( pFrame )
    {
      pFrame->AddHandler( fastdelegate::MakeDelegate( this, &ComponentDisplayerBase::OnFrameEvent ) );
    }
    Xtreme::XInput*     pInput = (Xtreme::XInput*)m_pEnvironment->Service( "Input" );
    if ( pInput )
    {
      m_pInputClass = pInput;
      pInput->AddListener( this );
    }
  }

      
      
  void ComponentDisplayerBase::OnUnset()
  {
    GR::RenderFrame*   pFrame = (GR::RenderFrame*)m_pEnvironment->Service( "WindowFrame" );
    if ( pFrame )
    {
      pFrame->RemoveHandler( fastdelegate::MakeDelegate( this, &ComponentDisplayerBase::OnFrameEvent ) );
    }
    Xtreme::XInput*     pInput = (Xtreme::XInput*)m_pEnvironment->Service( "Input" );
    if ( pInput )
    {
      pInput->RemoveListener( this );
    }
    m_pInputClass = NULL;
  }



  ComponentDisplayerBase::ComponentDisplayerBase() :
    m_ElapsedTimeSinceLastFrame( 0.0f ),
    m_pDefaultGUIFont( NULL ),
    m_Suspending( false ),
    m_NothingIsVisible( false ),
    m_pInputClass( NULL ),
    m_pRenderer( NULL ),
    m_pCurrentlyDrawnComponent( NULL )
  {
    s_pInstance = this;

    // Default-Farben
    SysColor( GUI::COL_SCROLLBAR,            0xff000000 | GFX::RGBToBGR( DefaultSysColor( 0 ) ) ); // COLOR_SCROLLBAR
    SysColor( GUI::COL_BACKGROUND,           0xff000000 | GFX::RGBToBGR( DefaultSysColor( 1 ) ) ); // COLOR_BACKGROUND
    SysColor( GUI::COL_ACTIVECAPTION,        0xff000000 | GFX::RGBToBGR( DefaultSysColor( 2 ) ) ); // COLOR_ACTIVECAPTION
    SysColor( GUI::COL_INACTIVECAPTION,      0xff000000 | GFX::RGBToBGR( DefaultSysColor( 3 ) ) ); // COLOR_INACTIVECAPTION
    SysColor( GUI::COL_MENU,                 0xff000000 | GFX::RGBToBGR( DefaultSysColor( 4 ) ) ); // COLOR_MENU
    SysColor( GUI::COL_WINDOW,               0xff000000 | GFX::RGBToBGR( DefaultSysColor( 5 ) ) ); // COLOR_WINDOW
    SysColor( GUI::COL_WINDOWFRAME,          0xff000000 | GFX::RGBToBGR( DefaultSysColor( 6 ) ) ); // COLOR_WINDOWFRAME
    SysColor( GUI::COL_MENUTEXT,             0xff000000 | GFX::RGBToBGR( DefaultSysColor( 7 ) ) ); // COLOR_MENUTEXT
    SysColor( GUI::COL_WINDOWTEXT,           0xff000000 | GFX::RGBToBGR( DefaultSysColor( 8 ) ) ); // COLOR_WINDOWTEXT
    SysColor( GUI::COL_CAPTIONTEXT,          0xff000000 | GFX::RGBToBGR( DefaultSysColor( 9 ) ) ); // COLOR_CAPTIONTEXT
    SysColor( GUI::COL_ACTIVEBORDER,         0xff000000 | GFX::RGBToBGR( DefaultSysColor( 10 ) ) ); // COLOR_ACTIVEBORDER

    SysColor( GUI::COL_INACTIVEBORDER,       0xff000000 | GFX::RGBToBGR( DefaultSysColor( 11 ) ) ); // COLOR_INACTIVEBORDER
    SysColor( GUI::COL_APPWORKSPACE,         0xff000000 | GFX::RGBToBGR( DefaultSysColor( 12 ) ) ); // COLOR_APPWORKSPACE
    SysColor( GUI::COL_HIGHLIGHT,            0xff000000 | GFX::RGBToBGR( DefaultSysColor( 13 ) ) ); // COLOR_HIGHLIGHT
    SysColor( GUI::COL_HIGHLIGHTTEXT,        0xff000000 | GFX::RGBToBGR( DefaultSysColor( 14 ) ) ); // COLOR_HIGHLIGHTTEXT
    SysColor( GUI::COL_BTNFACE,              0xff000000 | GFX::RGBToBGR( DefaultSysColor( 15 ) ) ); // COLOR_BTNFACE
    SysColor( GUI::COL_BTNSHADOW,            0xff000000 | GFX::RGBToBGR( DefaultSysColor( 16 ) ) ); // COLOR_BTNSHADOW
    SysColor( GUI::COL_GRAYTEXT,             0xff000000 | GFX::RGBToBGR( DefaultSysColor( 17 ) ) ); // COLOR_GRAYTEXT
    SysColor( GUI::COL_BTNTEXT,              0xff000000 | GFX::RGBToBGR( DefaultSysColor( 18 ) ) ); // COLOR_BTNTEXT
    SysColor( GUI::COL_INACTIVECAPTIONTEXT,  0xff000000 | GFX::RGBToBGR( DefaultSysColor( 19 ) ) ); // COLOR_INACTIVECAPTIONTEXT
    SysColor( GUI::COL_BTNHIGHLIGHT,         0xff000000 | GFX::RGBToBGR( DefaultSysColor( 20 ) ) ); // COLOR_BTNHIGHLIGHT
    SysColor( GUI::COL_3DDKSHADOW,           0xff000000 | GFX::RGBToBGR( DefaultSysColor( 21 ) ) ); // COLOR_3DDKSHADOW

    SysColor( GUI::COL_3DLIGHT,              0xff000000 | GFX::RGBToBGR( DefaultSysColor( 22 ) ) ); // COLOR_3DLIGHT
    SysColor( GUI::COL_INFOTEXT,             0xff000000 | GFX::RGBToBGR( DefaultSysColor( 23 ) ) ); // COLOR_INFOTEXT
    SysColor( GUI::COL_INFOBK,               0xff000000 | GFX::RGBToBGR( DefaultSysColor( 24 ) ) ); // COLOR_INFOBK
    SysColor( GUI::COL_HOTLIGHT,             0xff000000 | GFX::RGBToBGR( DefaultSysColor( 26 ) ) );//COLOR_HOTLIGHT ) ) );
    SysColor( GUI::COL_GRADIENTACTIVECAPTION, 0xff000000 | GFX::RGBToBGR( DefaultSysColor( 27 ) ) );//COLOR_GRADIENTACTIVECAPTION ) ) );
    SysColor( GUI::COL_GRADIENTINACTIVECAPTION, 0xff000000 | GFX::RGBToBGR( DefaultSysColor( 28 ) ) );//COLOR_GRADIENTINACTIVECAPTION ) ) );
    SysColor( GUI::COL_MENUHILIGHT,          0xff000000 | GFX::RGBToBGR( DefaultSysColor( 29 ) ) );//COLOR_MENUHILIGHT ) ) );
    SysColor( GUI::COL_MENUBAR,              0xff000000 | GFX::RGBToBGR( DefaultSysColor( 30 ) ) );//COLOR_MENUBAR ) ) );

    SysColor( GUI::COL_NC_AREA,              0xffffffff );
    SysColor( GUI::COL_THIN_BORDER,          0xff000000 );
    SysColor( GUI::COL_CURSOR,               0xff000000 | GFX::RGBToBGR( DefaultSysColor( 8 ) ) ); // COLOR_WINDOWTEXT
    SysColor( GUI::COL_HOTLIGHTTEXT,         0xff000000 | GFX::RGBToBGR( DefaultSysColor( 8 ) ) ); // COLOR_WINDOWTEXT

    SysColor( GUI::COL_HOTTIP_BACKGROUND,    0xc0ffffff );
    SysColor( GUI::COL_HOTTIP_TEXT,          0xff000000 );
  }



  ComponentDisplayerBase::~ComponentDisplayerBase()
  {
  }



  GR::u32 ComponentDisplayerBase::DefaultSysColor( GR::u32 ColorIndex )
  {
  #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    return ::GetSysColor( ColorIndex );
  #else
    switch ( ColorIndex )
    {
      case COL_SCROLLBAR:
        return 0xffc8c8c8;
      case COL_BACKGROUND:
        return 0xff1f6f89;
      case COL_ACTIVECAPTION:
        return 0xff99b4d1;
      case COL_INACTIVECAPTION:
        return 0xffbfcddb;
      case COL_MENU:
        return 0xfff0f0f0;
      case COL_WINDOW:
        return 0xffffffff;
      case COL_WINDOWFRAME:
        return 0xff646464;
      case COL_MENUTEXT:
        return 0xff000000;
      case COL_WINDOWTEXT:
        return 0xff000000;
      case COL_CAPTIONTEXT:
        return 0xff000000;
      case COL_ACTIVEBORDER:
        return 0xffb4b4b4;
      case COL_INACTIVEBORDER:
        return 0xfff4f7fc;
      case COL_APPWORKSPACE:
        return 0xffababab;
      case COL_HIGHLIGHT:
        return 0xff3399ff;
      case COL_HIGHLIGHTTEXT:
        return 0xffffffff;
      case COL_BTNFACE:
        return 0xfff0f0f0;
      case COL_BTNSHADOW:
        return 0xffa0a0a0;
      case COL_GRAYTEXT:
        return 0xff6d6d6d;
      case COL_BTNTEXT:
        return 0xff000000;
      case COL_INACTIVECAPTIONTEXT:
        return 0xff000000;
      case COL_BTNHIGHLIGHT:
        return 0xffffffff;
      case COL_3DDKSHADOW:
        return 0xff696969;
      case COL_3DLIGHT:
        return 0xffe3e3e3;
      case COL_INFOTEXT:
        return 0xff000000;
      case COL_INFOBK:
        return 0xffffffe1;
      case 26:
        return 0xff0066cc;
      case 27:
        return 0xffb9d1ea;
      case 28:
        return 0xffd7e4f2;
      case 29:
        return 0xff3399ff;
      case 30:
        return 0xfff0f0f0;
    }
    return 0xff000000;
  #endif
  }



  bool ComponentDisplayerBase::NothingIsVisible()
  {
    return m_NothingIsVisible;
  }



  void ComponentDisplayerBase::AddElementCreatedFromXMLListener( tElementCreatedFromXMLListener Listener )
  {
    m_ElementCreatedListeners.push_back( Listener );
  }



  void ComponentDisplayerBase::RemoveElementCreatedFromXMLListener( tElementCreatedFromXMLListener Listener )
  {
    m_ElementCreatedListeners.remove( Listener );
  }



  void ComponentDisplayerBase::Suspend( bool Suspend )
  {
    m_Suspending = Suspend;
  }



  void ComponentDisplayerBase::SysColor( GUI::eColorIndex colIndex, GR::u32 dwColor )
  {
    if ( colIndex < GUI::COL_LAST_ENTRY )
    {
      m_Colors[colIndex] = dwColor;
    }
  }



  GR::u32 ComponentDisplayerBase::GetSysColor( GUI::eColorIndex colIndex )
  {
    if ( colIndex < GUI::COL_LAST_ENTRY )
    {
      return m_Colors[colIndex];
    }
    return 0;
  }



  void ComponentDisplayerBase::StoreClipValues( int X, int Y, int Width, int Height, int XOffset, int YOffset )
  {
    m_ClipRects.push_back( GR::tRect( X, Y, Width, Height ) );
    m_Offsets.push_back( GR::tPoint( XOffset, YOffset ) );
  }



  void ComponentDisplayerBase::PopClipValues()
  {
    if ( !m_ClipRects.empty() )
    {
      GR::tRect&    rectClipping = m_ClipRects.back();

      SetClipping( rectClipping.Left,
                    rectClipping.Top,
                    rectClipping.Width(),
                    rectClipping.Height() );

      m_ClipRects.pop_back();
    }

    if ( !m_Offsets.empty() )
    {
      GR::tPoint&    ptOffset = m_Offsets.back();

      SetOffset( ptOffset.x, ptOffset.y );
      m_Offsets.pop_back();
    }
  }
      
      
      
  void ComponentDisplayerBase::GetStoredClipValues( int& X, int& Y, int& Width, int& Height, int& XOffset, int& YOffset )
  {
    if ( !m_ClipRects.empty() )
    {
      GR::tRect&    rectClipping = m_ClipRects.back();

      X      = rectClipping.Left;
      Y      = rectClipping.Top;
      Width  = rectClipping.Width();
      Height = rectClipping.Height();
    }
    if ( !m_Offsets.empty() )
    {
      GR::tPoint&   offset = m_Offsets.back();

      XOffset = offset.x;
      YOffset = offset.y;
    }
  }



  void ComponentDisplayerBase::DeleteComponent( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      return;
    }
    pComponent->Enable( false );
    pComponent->SetVisible( false );

    if ( pComponent->GetComponentParent() != NULL )
    {
      pComponent->GetComponentParent()->Delete( pComponent );
    }
    else
    {
      Delete( pComponent );
    }
  }



  void ComponentDisplayerBase::DeleteComponent( GR::u32 dwId )
  {
    Component*    pComponent = GetComponent( dwId );
    if ( pComponent )
    {
      DeleteComponent( pComponent );
    }
  }



  void ComponentDisplayerBase::DisplayComponent( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      return;
    }
    if ( !pComponent->IsVisible() )
    {
      return;
    }

    m_pCurrentlyDrawnComponent = pComponent;
    pComponent->Display( this );
    m_pCurrentlyDrawnComponent = NULL;
    DisplayChilds( pComponent );
  }



  void ComponentDisplayerBase::DisplayComponentFull( Component* pComponent, const GR::tPoint& Offset, const GR::tRect& ParentClipRect )
  {
    if ( pComponent == NULL )
    {
      return;
    }

    GR::tRect   rcParentClient( 0, 0, 500000, 500000 );

    if ( pComponent->GetComponentParent() )
    {
      pComponent->GetComponentParent()->GetClientRect( rcParentClient );
    }

    GR::tRect     ncAreaClip( Offset + pComponent->Position(), pComponent->Size() );

    if ( ( !ncAreaClip.Intersects( ParentClipRect ) )
    ||   ( !ncAreaClip.Intersects( GR::tRect( Offset, rcParentClient.Size() ) ) ) )
    {
      // completely outside!
      return;
    }
    ncAreaClip = ncAreaClip.Intersection( ParentClipRect );
    ncAreaClip = ncAreaClip.Intersection( GR::tRect( Offset, rcParentClient.Size() ) );

    SetClipping( ncAreaClip.Left, ncAreaClip.Top, ncAreaClip.Width(), ncAreaClip.Height() );

    GR::tPoint      offset( Offset + pComponent->Position() );
    SetOffset( offset.x, offset.y );

    m_pCurrentlyDrawnComponent = pComponent;
    pComponent->DisplayNonClient( this );
    m_pCurrentlyDrawnComponent = NULL;


    // display client area
    GR::tRect   rcClient;

    pComponent->GetClientRect( rcClient );

    GR::tPoint  ptClientOffset = pComponent->GetClientOffset();

    // clip to client rect
    GR::tRect   clientAreaClip( Offset + pComponent->Position() + ptClientOffset, rcClient.Size() );

    if ( ( !clientAreaClip.Intersects( ParentClipRect ) )
    &&   ( !clientAreaClip.Intersects( GR::tRect( Offset, rcParentClient.Size() ) ) ) )
    {
      // completely outside!
      return;
    }

    clientAreaClip = clientAreaClip.Intersection( ParentClipRect );
    clientAreaClip = clientAreaClip.Intersection( GR::tRect( Offset, rcParentClient.Size() ) );

    SetClipping( clientAreaClip.Left, clientAreaClip.Top, clientAreaClip.Width(), clientAreaClip.Height() );
    SetOffset( offset.x + ptClientOffset.x, offset.y + ptClientOffset.y );

    DisplayComponent( pComponent );
  }



  void ComponentDisplayerBase::DisplayChilds( Component* pParentComponent )
  {
    if ( pParentComponent == NULL )
    {
      return;
    }
    if ( pParentComponent->m_Components.empty() )
    {
      return;
    }

    PushClipValues();

    int     iX,
            iY,
            iWidth,
            iHeight,
            iXOffset,
            iYOffset;

    GetStoredClipValues( iX, iY, iWidth, iHeight, iXOffset, iYOffset );

    if ( ( iWidth == 0 )
    ||   ( iHeight == 0 ) )
    {
      PopClipValues();
      SetOffset( iXOffset, iYOffset );
      return;
    }

    Component*    pComponent = pParentComponent->m_Components.front();

    while ( true )
    {
      Component* pNextComponent = NULL;
      tListComponents::iterator   it( pParentComponent->m_Components.begin() );
      while ( ( it != pParentComponent->m_Components.end() )
      &&      ( *it != pComponent ) )
      {
        ++it;
      }
      if ( it == pParentComponent->m_Components.end() )
      {
        break;
      }
      ++it;
      if ( it != pParentComponent->m_Components.end() )
      {
        pNextComponent = *it;
      }

      // Hottips could be displayed twice because they shuffle the Z order during update
      if ( m_AlreadyHandledComponentsThisFrame.find( pComponent ) == m_AlreadyHandledComponentsThisFrame.end() )
      {
        pComponent->Update( m_ElapsedTimeSinceLastFrame );
        m_AlreadyHandledComponentsThisFrame.insert( pComponent );
        if ( pComponent->IsVisible() )
        {
          DisplayComponentFull( pComponent, GR::tPoint( iXOffset, iYOffset ), GR::tRect( iX, iY, iWidth, iHeight ) );
        }
      }

      // find next control (do not reuse iterator, list may have changed)
      pComponent = pNextComponent;
      /*
      tListComponents::iterator   it( pParentComponent->m_Components.begin() );
      while ( ( it != pParentComponent->m_Components.end() )
      &&      ( *it != pComponent ) )
      {
        ++it;
      }
      if ( it == pParentComponent->m_Components.end() )
      {
        break;
      }
      ++it;
      if ( it == pParentComponent->m_Components.end() )
      {
        break;
      }
      pComponent = *it;*/
    }

    PopClipValues();
    SetOffset( iXOffset, iYOffset );
  }



  void ComponentDisplayerBase::SetExtents( int Width, int Height )
  {
    GR::tPoint    trueSize = ScreenToVirtual( GR::tPoint( Width, Height ) );
    m_ClientRect.Size( trueSize.x, trueSize.y );

    ComponentEvent   Event( CET_EXTENTS_CHANGED );

    Event.Position.Set( Width, Height );

    NotifyAllComponents( Event );
  }



  void ComponentDisplayerBase::NotifyComponent( Component* pComponent, const GUI::ComponentEvent& Event )
  {
    pComponent->ProcessEvent( Event );

    tListComponents::iterator    it( pComponent->m_Components.begin() );
    while ( it != pComponent->m_Components.end() )
    {
      Component* pChildComponent = *it;

      NotifyComponent( pChildComponent, Event );
          
      it++;
    }
  }



  void ComponentDisplayerBase::NotifyAllComponents( const GUI::ComponentEvent& Event )
  {
    tListComponents::iterator    it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component* pChildComponent = *it;

      NotifyComponent( pChildComponent, Event );
          
      it++;
    }
  }



  void ComponentDisplayerBase::DisplayComponent( float ElapsedTime, Component* pRootComponent )
  {
    if ( pRootComponent == NULL )
    {
      return;
    }

    m_ElapsedTimeSinceLastFrame  = ElapsedTime;

    tListComponents::iterator    it( pRootComponent->m_Components.begin() );

    PushClipValues();

    int     iXOffset,
            iYOffset,
            iX,
            iY,
            iWidth,
            iHeight;

    GetStoredClipValues( iX, iY, iWidth, iHeight, iXOffset, iYOffset );

    while ( it != pRootComponent->m_Components.end() )
    {
      Component* pComponent = *it;

      pComponent->Update( m_ElapsedTimeSinceLastFrame );

      if ( !pComponent->IsVisible() )
      {
        ++it;
        continue;
      }

      int     iXA = iXOffset + pComponent->Position().x,
              iYA = iYOffset + pComponent->Position().y,
              iXB = iXOffset + pComponent->Position().x + pComponent->Width(),
              iYB = iYOffset + pComponent->Position().y + pComponent->Height();


      SetClipping( iXA, iYA, pComponent->Width(), pComponent->Height() );
      SetOffset( iXA, iYA );

      pComponent->DisplayNonClient( this );

      GR::tRect   rcClient;

      pComponent->GetClientRect( rcClient );

      GR::tPoint  ptClientOffset = pComponent->GetClientOffset();

      iXA = iXOffset + pComponent->Position().x + ptClientOffset.x;
      iYA = iYOffset + pComponent->Position().y + ptClientOffset.y;
      iXB = iXA + rcClient.Width();
      iYB = iYA + rcClient.Height();

      SetClipping( iXA, iYA, iXB - iXA, iYB - iYA );
      SetOffset( iXA, iYA );

      DisplayComponent( pComponent );
      it++;
    }

    PopClipValues();
    SetOffset( iXOffset, iYOffset );

    ProcessActions();
  }



  void ComponentDisplayerBase::UpdateComponent( GUI::Component* pComponent, const float ElapsedTime )
  {
    if ( pComponent == NULL )
    {
      return;
    }

    pComponent->Update( ElapsedTime );

    size_t    curIndex = 0;
    size_t    curSize = pComponent->m_Components.size();

    while ( curIndex < curSize )
    {
      GUI::Component* pChildComponent = pComponent->m_Components[curIndex];

      UpdateComponent( pChildComponent, ElapsedTime );

      if ( curSize != pComponent->m_Components.size() )
      {
        // do NOT inc current index, the next control might have been removed
        curSize = pComponent->m_Components.size();
      }
      else
      {
        ++curIndex;
      }
    }
    ( (IEventListener<ComponentEvent>*)pComponent )->ProcessQueue();
    pComponent->ProcessComponentEventQueue();
  }



  void ComponentDisplayerBase::UpdateAllControls( const float ElapsedTime )
  {
    m_ElapsedTimeSinceLastFrame  = ElapsedTime;

    size_t    curIndex = 0;
    size_t    curSize = m_Components.size();

    while ( curIndex < curSize )
    {
      GUI::Component* pComponent = m_Components[curIndex];

      UpdateComponent( pComponent, ElapsedTime );
      if ( curSize != m_Components.size() )
      {
        // do NOT inc current index, the next control might have been removed
        curSize = m_Components.size();
      }
      else
      {
        ++curIndex;
      }
    }
    ProcessComponentEventQueue();
    DeleteDanglingControls();
    // TODO - can that stay here?
    m_AlreadyHandledComponentsThisFrame.clear();
  }



  void ComponentDisplayerBase::DisplayAllControls()
  {
    if ( m_Components.empty() )
    {
      return;
    }
    m_AlreadyHandledComponentsThisFrame.clear();

    PushClipValues();

    int     iXOffset,
            iYOffset,
            iX,
            iY,
            iWidth,
            iHeight;

    GetStoredClipValues( iX, iY, iWidth, iHeight, iXOffset, iYOffset );

    Component* pComponent = m_Components.front();

    while ( true )
    {
      if ( m_AlreadyHandledComponentsThisFrame.find( pComponent ) == m_AlreadyHandledComponentsThisFrame.end() )
      {
        pComponent->Update( m_ElapsedTimeSinceLastFrame );
        m_AlreadyHandledComponentsThisFrame.insert( pComponent );
        if ( pComponent->IsVisible() )
        {
          int     iXA = iXOffset + pComponent->Position().x,
                  iYA = iYOffset + pComponent->Position().y,
                  iXB = iXOffset + pComponent->Position().x + pComponent->Width(),
                  iYB = iYOffset + pComponent->Position().y + pComponent->Height();


          SetClipping( iXA, iYA, pComponent->Width(), pComponent->Height() );
          SetOffset( iXOffset + pComponent->Position().x,
                      iYOffset + pComponent->Position().y );

          if ( !m_NothingIsVisible )
          {
            pComponent->DisplayNonClient( this );

            GR::tRect   rcClient;

            pComponent->GetClientRect( rcClient );

            GR::tPoint  ptClientOffset = pComponent->GetClientOffset();

            iXA = iXOffset + pComponent->Position().x + ptClientOffset.x;
            iYA = iYOffset + pComponent->Position().y + ptClientOffset.y;
            iXB = iXA + rcClient.Width();
            iYB = iYA + rcClient.Height();

            if ( ( iXB - iXA > 0 )
            &&   ( iYB - iYA > 0 )
            &&   ( iXB > 0 )
            &&   ( iYB > 0 ) )
            {
              SetClipping( iXA, iYA, iXB - iXA, iYB - iYA );
              SetOffset( iXA, iYA );

              if ( !m_NothingIsVisible )
              {
                DisplayComponent( pComponent );
              }
            }
          }
        }
      }
      // find next component -> butt ugly re-check from start of container, since iterators are hosed if the list is modified
      tListComponents::iterator    it( m_Components.begin() );

      while ( ( it != m_Components.end() )
      &&      ( *it != pComponent ) )
      {
        ++it;
      }
      if ( it == m_Components.end() )
      {
        break;
      }
      ++it;
      if ( it == m_Components.end() )
      {
        break;
      }
      pComponent = *it;
    }

    PopClipValues();
    SetOffset( iXOffset, iYOffset );

    ProcessActions();
  }

      
      
  void ComponentDisplayerBase::DisplayComponentFromParent( Component* pParentComponent )
  {
    PushClipValues();

    int     iXOffset,
            iYOffset,
            iX,
            iY,
            iWidth,
            iHeight;

    GetStoredClipValues( iX, iY, iWidth, iHeight, iXOffset, iYOffset );


    pParentComponent->Update( m_ElapsedTimeSinceLastFrame );

    if ( !pParentComponent->IsVisible() )
    {
      PopClipValues();
      return;
    }

    int     iXA = iXOffset + pParentComponent->Position().x,
            iYA = iYOffset + pParentComponent->Position().y,
            iXB = iXOffset + pParentComponent->Position().x + pParentComponent->Width(),
            iYB = iYOffset + pParentComponent->Position().y + pParentComponent->Height();


    SetClipping( iXA, iYA, pParentComponent->Width(), pParentComponent->Height() );
    SetOffset( iXA, iYA );

    if ( !m_NothingIsVisible )
    {
      m_pCurrentlyDrawnComponent = pParentComponent;
      pParentComponent->DisplayNonClient( this );
      m_pCurrentlyDrawnComponent = NULL;

      GR::tRect   rcClient;

      pParentComponent->GetClientRect( rcClient );

      GR::tPoint  ptClientOffset = pParentComponent->GetClientOffset();

      iXA = iXOffset + pParentComponent->Position().x + ptClientOffset.x;
      iYA = iYOffset + pParentComponent->Position().y + ptClientOffset.y;
      iXB = iXA + rcClient.Width();
      iYB = iYA + rcClient.Height();

      if ( ( iXB - iXA > 0 )
      &&   ( iYB - iYA > 0 )
      &&   ( iXB > 0 )
      &&   ( iYB > 0 ) )
      {
        SetClipping( iXA, iYA, iXB - iXA, iYB - iYA );
        SetOffset( iXA, iYA );

        if ( !m_NothingIsVisible )
        {
          DisplayComponent( pParentComponent );
        }
      }
    }

    PopClipValues();
    SetOffset( iXOffset, iYOffset );

    ProcessActions();
  }

      
      
  void ComponentDisplayerBase::ProcessActions()
  {
    DeleteDanglingControls();
  }



  GR::tPoint ComponentDisplayerBase::ScreenToVirtual( const GR::tPoint& ScreenCoordinates )
  {
    if ( ( m_VirtualSize.x == 0 )
    ||   ( m_PhysicalSize.x == 0 ) )
    {
      return ScreenCoordinates;
    }
    return GR::tPoint( ( ScreenCoordinates.x * m_VirtualSize.x ) / m_PhysicalSize.x,
                        ( ScreenCoordinates.y * m_VirtualSize.y ) / m_PhysicalSize.y );
  }



  GR::tPoint ComponentDisplayerBase::VirtualToScreen( const GR::tPoint& VirtualCoordinates )
  {
    if ( ( m_VirtualSize.x == 0 )
    ||   ( m_PhysicalSize.x == 0 ) )
    {
      return VirtualCoordinates;
    }
    return GR::tPoint( ( VirtualCoordinates.x * m_PhysicalSize.x ) / m_VirtualSize.x,
                        ( VirtualCoordinates.y * m_PhysicalSize.y ) / m_VirtualSize.y );
  }



  void ComponentDisplayerBase::VirtualToScreen( GR::tRect& VirtualCoordinates )
  {
    if ( ( m_VirtualSize.x == 0 )
    ||   ( m_PhysicalSize.x == 0 ) )
    {
      return;
    }

    int     newX = ( VirtualCoordinates.Left * m_PhysicalSize.x ) / m_VirtualSize.x;
    int     newY = ( VirtualCoordinates.Top * m_PhysicalSize.y ) / m_VirtualSize.y;
    int     width = ( VirtualCoordinates.Right * m_PhysicalSize.x ) / m_VirtualSize.x - newX;
    int     height = ( VirtualCoordinates.Bottom * m_PhysicalSize.y ) / m_VirtualSize.y - newY;

    VirtualCoordinates.Left   = newX;
    VirtualCoordinates.Top    = newY;
    VirtualCoordinates.Right  = newX + width;
    VirtualCoordinates.Bottom = newY + height;
  }



  void ComponentDisplayerBase::VirtualToScreen( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height )
  {
    if ( ( m_VirtualSize.x == 0 )
    ||   ( m_PhysicalSize.x == 0 ) )
    {
      return;
    }
    int     newX = ( X * m_PhysicalSize.x ) / m_VirtualSize.x;
    int     newY = ( Y * m_PhysicalSize.y ) / m_VirtualSize.y;
    Width   = ( ( X + Width ) * m_PhysicalSize.x ) / m_VirtualSize.x - newX;
    Height  = ( ( Y + Height ) * m_PhysicalSize.y ) / m_VirtualSize.y - newY;

    X = newX;
    Y = newY;
  }



  int ComponentDisplayerBase::TextLength( Interface::IFont* pFont, const GR::String& Text )
  {
    int     length = pFont->TextLength( Text );

    GR::tPoint    dummy( length, 0 );

    VirtualToScreen( dummy );

    return dummy.x;
  }



  int ComponentDisplayerBase::TextHeight( Interface::IFont* pFont, const GR::String& Text )
  {
    int     length = pFont->TextHeight( Text );

    GR::tPoint    dummy( 0, length );

    VirtualToScreen( dummy );

    return dummy.y;
  }



  void ComponentDisplayerBase::ScreenToVirtual( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height )
  {
    if ( ( m_VirtualSize.x == 0 )
    ||   ( m_PhysicalSize.x == 0 ) )
    {
      return;
    }
    int     newX = ( X * m_VirtualSize.x ) / m_PhysicalSize.x;
    int     newY = ( Y * m_VirtualSize.y ) / m_PhysicalSize.y;
    Width   = ( ( X + Width ) * m_VirtualSize.x ) / m_PhysicalSize.x - newX;
    Height  = ( ( Y + Height ) * m_VirtualSize.y ) / m_PhysicalSize.y - newY;

    X = newX;
    Y = newY;
  }



  bool ComponentDisplayerBase::ProcessEvent( const Xtreme::tInputEvent& Event )
  {
    if ( m_Suspending )
    {
      return true;
    }
    if ( Event.Type == Xtreme::tInputEvent::IE_MOUSE_UPDATE )
    {
      ComponentEvent   inputEvent( CET_MOUSE_UPDATE );

      inputEvent.MouseButtons     = (GR::u32)Event.Param2;
      inputEvent.Position.x       = (int)( (int)Event.Param1 >> 16 );
      inputEvent.Position.y       = (GR::i16)( Event.Param1 & 0xffff );
      inputEvent.Position         = ScreenToVirtual( inputEvent.Position );

      if ( m_ExternalArea.Width() != 0 )
      {
        inputEvent.Position = GR::tPoint( ( inputEvent.Position.x * m_ExternalArea.Width() ) / m_PhysicalSize.x - m_ExternalArea.Left,
                                          ( inputEvent.Position.y * m_ExternalArea.Height() ) / m_PhysicalSize.y - m_ExternalArea.Top );
      }

      ProcessEvent( inputEvent );
    }
    else if ( Event.Type == Xtreme::tInputEvent::IE_MOUSEWHEEL_DOWN )
    {
      ComponentEvent    inputEvent( CET_MOUSE_WHEEL );
      inputEvent.Value            = 1;
      inputEvent.Position.x       = (int)( (int)Event.Param1 >> 16 );
      inputEvent.Position.y       = (GR::i16)( Event.Param1 & 0xffff );
      inputEvent.Position         = ScreenToVirtual( inputEvent.Position );

      if ( m_ExternalArea.Width() != 0 )
      {
        inputEvent.Position = GR::tPoint( ( inputEvent.Position.x * m_ExternalArea.Width() ) / m_PhysicalSize.x - m_ExternalArea.Left,
                                          ( inputEvent.Position.y * m_ExternalArea.Height() ) / m_PhysicalSize.y - m_ExternalArea.Top );
      }

      ProcessEvent( inputEvent );
    }
    else if ( Event.Type == Xtreme::tInputEvent::IE_MOUSEWHEEL_UP )
    {
      ComponentEvent   inputEvent( CET_MOUSE_WHEEL );
      inputEvent.Value            = -1;
      inputEvent.Position.x       = (int)( (int)Event.Param1 >> 16 );
      inputEvent.Position.y       = (GR::i16)( Event.Param1 & 0xffff );
      inputEvent.Position         = ScreenToVirtual( inputEvent.Position );

      if ( m_ExternalArea.Width() != 0 )
      {
        inputEvent.Position = GR::tPoint( ( inputEvent.Position.x * m_ExternalArea.Width() ) / m_PhysicalSize.x - m_ExternalArea.Left,
                                          ( inputEvent.Position.y * m_ExternalArea.Height() ) / m_PhysicalSize.y - m_ExternalArea.Top );
      }

      ProcessEvent( inputEvent );
    }
    else if ( Event.Type == Xtreme::tInputEvent::IE_KEY_UP )
    {
      ComponentEvent   inputEvent( CET_KEY_UP );

      inputEvent.Character  = (GR::u16)Event.Param1;
      inputEvent.Value      = (GR::i32)Event.Param2;
      ProcessEvent( inputEvent );
    }
    else if ( Event.Type == Xtreme::tInputEvent::IE_KEY_DOWN )
    {
      ComponentEvent   inputEvent( CET_KEY_DOWN );

      inputEvent.Character  = (GR::u16)Event.Param1;
      inputEvent.Value      = (GR::i32)Event.Param2;
      ProcessEvent( inputEvent );
    }
    else if ( Event.Type == Xtreme::tInputEvent::IE_CHAR_ENTERED )
    {
      ComponentEvent   inputEvent( CET_KEY_PRESSED );

      inputEvent.Character = (GR::u16)Event.Param2;
      ProcessEvent( inputEvent );
    }
    return true;
  }



  bool ComponentDisplayerBase::IsVisible( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      return false;
    }
    while ( pComponent )
    {
      if ( !pComponent->IsVisible() )
      {
        return false;
      }
      pComponent = pComponent->GetComponentParent();
    }
    return true;
  }



  bool ComponentDisplayerBase::ProcessEventOnComponent( const GUI::ComponentEvent& Event, Component* pComponent )
  {
    if ( m_ContainerProcessingDisabled )
    {
      return true;
    }

    if ( ( m_pCapturingComponent )
    &&   ( m_pCapturingComponent != pComponent ) )
    {
      if ( ( Event.Type == CET_MOUSE_UPDATE )
      ||   ( Event.Type == CET_MOUSE_UP )
      ||   ( Event.Type == CET_MOUSE_DOWN )
      ||   ( Event.Type == CET_MOUSE_WHEEL )
      ||   ( Event.Type == CET_MOUSE_RUP )
      ||   ( Event.Type == CET_MOUSE_RDOWN ) )
      {
        ComponentEvent     newEvent( Event );

        ScreenToNonClient( newEvent.Position, m_pCapturingComponent );

        if ( m_pCapturingComponent->IsMouseInsideNonClientArea( newEvent.Position ) )
        {
          if ( !m_pCapturingComponent->IsMouseInside() )
          {
            m_pCapturingComponent->m_ComponentFlags |= GUI::COMPFT_MOUSE_INSIDE;
            ComponentEvent     mouseinEvent( CET_MOUSE_IN );
            mouseinEvent.MouseButtons = newEvent.MouseButtons;
            mouseinEvent.Position = newEvent.Position - m_pCapturingComponent->m_ClientRect.Position();
            m_pCapturingComponent->ProcessEvent( mouseinEvent );
          }
        }
        else
        {
          if ( m_pCapturingComponent->IsMouseInside() )
          {
            m_pCapturingComponent->m_ComponentFlags &= ~GUI::COMPFT_MOUSE_INSIDE;
            ComponentEvent    newEvent( CET_MOUSE_OUT );
            m_pCapturingComponent->ProcessEvent( newEvent );
          }
        }
        if ( m_pCapturingComponent->ProcessEvent( newEvent ) )
        {
          return true;
        }
        return true;
      }
    }

    if ( ( m_pFocusedComponent )
    &&   ( !m_pFocusedComponent->IsIndirectDisabled() )
    &&   ( IsVisible( m_pFocusedComponent ) ) )
    {
      if ( ( Event.Type == CET_KEY_PRESSED )
      ||   ( Event.Type == CET_KEY_DOWN )
      ||   ( Event.Type == CET_KEY_UP ) )
      {
        if ( ( !m_pFocusedComponent->IsVisible() )
        ||   ( !m_pFocusedComponent->IsEnabled() ) )
        {
          return true;
        }
        if ( ( Event.Type == CET_KEY_DOWN )
        &&   ( Event.Character == Xtreme::KEY_TAB ) )
        {
          // n‰chstes Element fokussieren
          Component* pOtherComponent = NULL;

          if ( m_pFocusedComponent )
          {
            // zuerst im Parent des fokussierten Elements nachsehen
            if ( m_pFocusedComponent->GetParentContainer() )
            {
              pOtherComponent = m_pFocusedComponent->GetParentContainer()->FindNextTabComponent( m_pFocusedComponent, m_pInputClass->ShiftPressed() );
            }
          }
          if ( pOtherComponent == NULL )
          {
            pOtherComponent = FindNextTabComponent( m_pFocusedComponent, m_pInputClass->ShiftPressed() );
          }
          if ( pOtherComponent != m_pFocusedComponent )
          {
            SetFocus( pOtherComponent );
            return true;
          }
        }
        if ( m_pFocusedComponent->ProcessEvent( Event ) )
        {
          return true;
        }
      }
      if ( Event.Type == CET_KEY_PRESSED )
      {
        if ( ( Event.Character == Xtreme::KEY_DOWN )
        ||   ( Event.Character == Xtreme::KEY_UP )
        ||   ( Event.Character == Xtreme::KEY_LEFT )
        ||   ( Event.Character == Xtreme::KEY_RIGHT ) )
        {
          Component* pOtherComponent = FindNextTabComponentByDir( this, m_pFocusedComponent, Event.Character );
          if ( pOtherComponent != m_pFocusedComponent )
          {
            SetFocus( pOtherComponent );
            return true;
          }
        }
      }
    }

    if ( Event.Type == CET_MOUSE_UPDATE )
    {
      GR::u32     PrevLastMouseButtons = m_LastMouseButtons;

      m_LastMouseButtons = Event.MouseButtons;

      if ( ( PrevLastMouseButtons & 1 )
      &&   ( ( Event.MouseButtons & 1 ) == 0 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_UP );

        newEvent.Position = Event.Position;
        newEvent.MouseButtons = Event.MouseButtons;

        pComponent->ProcessEvent( newEvent );
      }
      else if ( ( ( PrevLastMouseButtons & 1 ) == 0 )
      &&        ( Event.MouseButtons & 1 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_DOWN );

        newEvent.Position = Event.Position;
        newEvent.MouseButtons = Event.MouseButtons;

        pComponent->ProcessEvent( newEvent );
      }
      if ( ( PrevLastMouseButtons & 2 )
      &&   ( ( Event.MouseButtons & 2 ) == 0 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_RUP );

        newEvent.Position = Event.Position;
        newEvent.MouseButtons = Event.MouseButtons;

        pComponent->ProcessEvent( newEvent );
      }
      else if ( ( ( PrevLastMouseButtons & 2 ) == 0 )
      &&        ( Event.MouseButtons & 2 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_RDOWN );

        newEvent.Position = Event.Position;
        newEvent.MouseButtons = Event.MouseButtons;

        pComponent->ProcessEvent( newEvent );
      }
    }
    return pComponent->ProcessEvent( Event );
  }



  bool ComponentDisplayerBase::ProcessEvent( const GUI::ComponentEvent& Event )
  {
    if ( m_ContainerProcessingDisabled )
    {
      return true;
    }
    if ( ( m_pFocusedComponent )
    &&   ( !m_pFocusedComponent->IsIndirectDisabled() )
    &&   ( IsVisible( m_pFocusedComponent ) ) )
    {
      if ( ( Event.Type == CET_KEY_PRESSED )
      ||   ( Event.Type == CET_KEY_DOWN )
      ||   ( Event.Type == CET_KEY_UP ) )
      {
        if ( ( !m_pFocusedComponent->IsVisible() )
        ||   ( !m_pFocusedComponent->IsEnabled() ) )
        {
          return true;
        }
        if ( ( Event.Type == CET_KEY_DOWN )
        &&   ( Event.Character == Xtreme::KEY_TAB ) )
        {
          // n‰chstes Element fokussieren
          Component*   pOtherComponent = NULL;

          if ( m_pFocusedComponent )
          {
            // zuerst im Parent des fokussierten Elements nachsehen
            if ( m_pFocusedComponent->GetParentContainer() )
            {
              pOtherComponent = m_pFocusedComponent->GetParentContainer()->FindNextTabComponent( m_pFocusedComponent, m_pInputClass->ShiftPressed() );
            }
          }
          if ( pOtherComponent == NULL )
          {
            pOtherComponent = FindNextTabComponent( m_pFocusedComponent, m_pInputClass->ShiftPressed() );
          }
          if ( pOtherComponent != m_pFocusedComponent )
          {
            SetFocus( pOtherComponent );
            return true;
          }
        }

        if ( m_pFocusedComponent->ProcessEvent( Event ) )
        {
          return true;
        }
      }
      if ( Event.Type == CET_KEY_PRESSED )
      {
        if ( ( Event.Character == Xtreme::KEY_DOWN )
        ||   ( Event.Character == Xtreme::KEY_UP )
        ||   ( Event.Character == Xtreme::KEY_LEFT )
        ||   ( Event.Character == Xtreme::KEY_RIGHT ) )
        {
          Component*   pOtherComponent = FindNextTabComponentByDir( this, m_pFocusedComponent, Event.Character );
          if ( pOtherComponent != m_pFocusedComponent )
          {
            SetFocus( pOtherComponent );
            return true;
          }
        }
      }
    }

    if ( Event.Type == CET_MOUSE_UPDATE )
    {
      GR::u32     PrevLastMouseButtons = m_LastMouseButtons;

      m_LastMouseButtons = Event.MouseButtons;

      if ( ( PrevLastMouseButtons & 1 )
      &&   ( ( Event.MouseButtons & 1 ) == 0 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_UP );

        newEvent.Position       = Event.Position;
        newEvent.MouseButtons   = Event.MouseButtons;

        ComponentContainer::ProcessEvent( newEvent );
      }
      else if ( ( ( PrevLastMouseButtons & 1 ) == 0 )
      &&        ( Event.MouseButtons & 1 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_DOWN );

        newEvent.Position       = Event.Position;
        newEvent.MouseButtons   = Event.MouseButtons;

        ComponentContainer::ProcessEvent( newEvent );
      }
      if ( ( PrevLastMouseButtons & 2 )
      &&   ( ( Event.MouseButtons & 2 ) == 0 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_RUP );

        newEvent.Position       = Event.Position;
        newEvent.MouseButtons   = Event.MouseButtons;

        ComponentContainer::ProcessEvent( newEvent );
      }
      else if ( ( ( PrevLastMouseButtons & 2 ) == 0 )
      &&        ( Event.MouseButtons & 2 ) )
      {
        ComponentEvent     newEvent( CET_MOUSE_RDOWN );

        newEvent.Position       = Event.Position;
        newEvent.MouseButtons   = Event.MouseButtons;

        ComponentContainer::ProcessEvent( newEvent );
      }
      //m_LastMouseButtons = Event.m_MouseButtons;
    }
    return ComponentContainer::ProcessEvent( Event );
  }



  void ComponentDisplayerBase::SetDefaultFont( Interface::IFont* pFont )
  {
    m_pDefaultGUIFont = pFont;
  }



  Interface::IFont* ComponentDisplayerBase::DefaultFont() const
  {
    return m_pDefaultGUIFont;
  }



  Component* ComponentDisplayerBase::ComponentFromXML( GR::Strings::XMLElement* pElement )
  {
    if ( pElement == NULL )
    {
      return NULL;
    }

    GR::String     compClass = pElement->Attribute( "Class" );
    if ( compClass.empty() )
    {
      return NULL;
    }
    // overriding custom class
    if ( pElement->HasAttribute( "CustomClass" ) )
    {
      compClass = pElement->Attribute( "CustomClass" );
    }
    Component*   pComponent = (Component*)CreateObject( compClass );
    if ( pComponent == NULL )
    {
      dh::Log( "ComponentFromXML: Unregistered class %s", compClass.c_str() );
      return NULL;
    }
    pComponent->ParseXML( pElement, GR::Service::Environment::Instance() );

    std::list<tElementCreatedFromXMLListener>::const_iterator   itXMLListeners( m_ElementCreatedListeners.begin() );
    while ( itXMLListeners != m_ElementCreatedListeners.end() )
    {
      const tElementCreatedFromXMLListener& Listener( *itXMLListeners );

      Listener( pElement, pComponent );

      ++itXMLListeners;
    }

    GR::Strings::XML::iterator    it( pElement->FirstChild() );
    while ( it != GR::Strings::XML::iterator() )
    {
      GR::Strings::XMLElement*  pChild = *it;

      Component*   pChildComponent = ComponentFromXML( pChild );
      if ( pChildComponent )
      {
        pComponent->Add( pChildComponent );
      }

      it = it.next_sibling();
    }
    return pComponent;
  }



  GR::Strings::XMLElement* ComponentDisplayerBase::ComponentToXML( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      return NULL;
    }
    GR::Strings::XMLElement*    pElement = new GR::Strings::XMLElement( "Component" );

    pComponent->PropertiesToXML( pElement );

    pElement->SetAttribute( "Class", pComponent->Class() );

    // jetzt die Childs
    tListComponents::iterator   it( pComponent->m_Components.begin() );
    while ( it != pComponent->m_Components.end() )
    {
      Component*   pChild = *it;

      if ( !( pChild->Flags() & GUI::COMPFT_NOT_SERIALIZABLE ) )
      {
        pElement->InsertChild( ComponentToXML( pChild ) );
      }

      ++it;
    }

    return pElement;

  }



  Component* ComponentDisplayerBase::FindNextTabComponentByDir( GUI::ComponentContainer* pParentComponent, Component* pOldFocusedComponent, GR::u16 wChar )
  {
    if ( pParentComponent == NULL )
    {
      return NULL;
    }
    if ( pParentComponent->m_Components.empty() )
    {
      return NULL;
    }

    float   angle = 0.0f;

    switch ( wChar )
    {
      case Xtreme::KEY_LEFT:
        angle = 180.0f;
        break;
      case Xtreme::KEY_UP:
        angle = 90.0f;
        break;
      case Xtreme::KEY_DOWN:
        angle = 270.0f;
        break;
    }

    Component*    pBestMatch = NULL;

    GR::tRect     rcMyComponent;

    if ( pOldFocusedComponent )
    {
      pOldFocusedComponent->GetWindowRect( rcMyComponent );
    }

    float         bestDistance = 100000.0f;

    tListComponents::iterator   it( pParentComponent->m_Components.begin() );
    while ( it != pParentComponent->m_Components.end() )
    {
      Component*   pOtherComponent = *it;

      if ( pOtherComponent->IsVisible() )
      {
        if ( ( !( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP ) )
        &&   ( pOtherComponent->IsEnabled() ) )
        {
          // pr¸fen, ob es ein Child gibt, das Tab-Stop hat!
          Component*   pChild = FindNextTabComponentByDir( pOtherComponent, pOldFocusedComponent, wChar );
          if ( pChild )
          {
            pOtherComponent = pChild;
          }
          else
          {
            ++it;
            continue;
          }
        }

        if ( pOtherComponent != pOldFocusedComponent )
        {
          GR::tRect   rc;

          pOtherComponent->GetWindowRect( rc );

          float   distance = (float)( rc.CenterX() - rcMyComponent.CenterX() ) * ( rc.CenterX() - rcMyComponent.CenterX() )
                                  + ( rc.CenterY() - rcMyComponent.CenterY() ) * ( rc.CenterY() - rcMyComponent.CenterY() );

          float   localAngle = atan2f( (float)( rcMyComponent.CenterY() - rc.CenterY() ), 
                                      -(float)( rcMyComponent.CenterX() - rc.CenterX() ) ) * 180.0f / 3.1415926f;

          while ( localAngle < 0.0f )
          {
            localAngle += 360.0f;
          }

          if ( ( fabsf( localAngle - angle ) <= 45.0f )
          ||   ( fabsf( localAngle - angle ) >= 315.0f ) )
          {
            // im erlaubten Bereich
            if ( distance < bestDistance )
            {
              pBestMatch = pOtherComponent;
              bestDistance = distance;
            }
          }
        }
      }

      ++it;
    }

    if ( pBestMatch == NULL )
    {
      return pOldFocusedComponent;
    }
    return pBestMatch;
  }



  void ComponentDisplayerBase::SetScreenSizes( const GR::tPoint& PhysicalSize, const GR::tPoint& VirtualSize )
  {
    m_VirtualSize = VirtualSize;
    m_PhysicalSize = PhysicalSize;

    m_ClientRect.Set( 0, 0, PhysicalSize.x, PhysicalSize.y );
  }



  GR::tPoint ComponentDisplayerBase::VirtualSize() const
  {
    return m_VirtualSize;
  }



  GR::tPoint ComponentDisplayerBase::PhysicalSize() const
  {
    return m_PhysicalSize;
  }



  void ComponentDisplayerBase::SetExternalArea( const GR::tRect& Area )
  {
    m_ExternalArea = Area;
  }



  void ComponentDisplayerBase::SetDefaultTextureSection( GUI::eBorderType Type, const XTextureSection& Section, GR::u32 ColorKey )
  {
    if ( Type >= GUI::BT_EDGE_LAST_ENTRY )
    {
      return;
    }
    m_DefaultTextureSection[Type] = std::make_pair( Section, ColorKey );
  }
      
      
      
  void ComponentDisplayerBase::SetCustomTextureSection( const GR::u32 Type, const XTextureSection& Section, GR::u32 ColorKey )
  {
    m_CustomTextureSections[Type] = std::make_pair( Section, ColorKey );
  }



  ComponentDisplayerBase& ComponentDisplayerBase::Instance()
  {
    return *s_pInstance;
  }



  void ComponentDisplayerBase::Invalidate( Component* pComponent )
  {
  }



}
