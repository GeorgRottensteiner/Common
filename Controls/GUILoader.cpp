#include "GUILoader.h"

#include <GR/Database/DatabaseTable.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <Xtreme/XTextureSection.h>



namespace GUI
{


  Loader::Loader( ComponentDisplayerBase& GUI ) :
    m_pTable( NULL ),
    m_pVar( NULL ),
    m_GUI( GUI )
  {
    GR::Service::Environment::Instance().SetService( "GUILoader", this );
  }



  Loader::~Loader()
  {
    GR::Service::Environment::Instance().RemoveService( "GUILoader" );
  }



  bool Loader::Initialize( IIOStream& ioIn, GR::Database::Table* pTable, LocalRegistry* pLocalRegistry )
  {
    if ( !m_XMLGUI.Load( ioIn ) )
    {
      return false;
    }
    m_pTable = pTable;
    m_pVar   = pLocalRegistry;

    m_pVar->AddHandler( fastdelegate::MakeDelegate( this, &Loader::OnVarEvent ) );
    return true;
  }



  bool Loader::LoadFromXML( IIOStream& ioIn )
  {

    return m_XMLGUI.Load( ioIn );

  }



  bool Loader::Initialize( GR::Strings::XMLElement* pGUIRootAsset, GR::Database::Table* pTable, LocalRegistry* pLocalRegistry )
  {
    m_XMLGUI.Clear();

    m_XMLGUIRoot = *pGUIRootAsset;

    GR::Strings::XML::iterator    it( pGUIRootAsset->FirstChild() );
    while ( it != GR::Strings::XML::iterator() )
    {
      GR::Strings::XMLElement*  pXMLChild = *it;

      m_XMLGUI.InsertChild( pXMLChild->Clone() );

      it = it.next_sibling();
    }
    m_pTable = pTable;
    m_pVar   = pLocalRegistry;

    StoreDefaultValues();

    m_pVar->AddHandler( fastdelegate::MakeDelegate( this, &Loader::OnVarEvent ) );
    return true;
  }



  void Loader::ApplyCustomSection( GR::Strings::XMLElement* pElement, const GR::String& AttributeName, GUI::eCustomTextureSections CustomSection )
  {
    if ( pElement->HasAttribute( AttributeName ) )
    {
      m_CustomSections[CustomSection] = pElement->Attribute( AttributeName );
    }
  }



  GR::u32 Loader::GetSysColor( GR::u32 ColorIndex )
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



  void Loader::StoreDefaultValues()
  {
    m_DefaultColors[GUI::COL_SCROLLBAR]           = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_SCROLLBAR ) );
    m_DefaultColors[GUI::COL_BACKGROUND]          = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_BACKGROUND ) );
    m_DefaultColors[GUI::COL_ACTIVECAPTION]       = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_ACTIVECAPTION ) );
    m_DefaultColors[GUI::COL_INACTIVECAPTION]     = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_INACTIVECAPTION ) );
    m_DefaultColors[GUI::COL_MENU]                = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_MENU ) );
    m_DefaultColors[GUI::COL_WINDOW]              = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_WINDOW ) );
    m_DefaultColors[GUI::COL_WINDOWFRAME]         = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_WINDOWFRAME ) );
    m_DefaultColors[GUI::COL_MENUTEXT]            = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_MENUTEXT ) );
    m_DefaultColors[GUI::COL_WINDOWTEXT]          = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_WINDOWTEXT ) );
    m_DefaultColors[GUI::COL_CAPTIONTEXT]         = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_CAPTIONTEXT ) );
    m_DefaultColors[GUI::COL_ACTIVEBORDER]        = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_ACTIVEBORDER ) );

    m_DefaultColors[GUI::COL_INACTIVEBORDER]      = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_INACTIVEBORDER ) );
    m_DefaultColors[GUI::COL_APPWORKSPACE]        = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_APPWORKSPACE ) );
    m_DefaultColors[GUI::COL_HIGHLIGHT]           = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_HIGHLIGHT ) );
    m_DefaultColors[GUI::COL_HIGHLIGHTTEXT]       = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_HIGHLIGHTTEXT ) );
    m_DefaultColors[GUI::COL_BTNFACE]             = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_BTNFACE ) );
    m_DefaultColors[GUI::COL_BTNSHADOW]           = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_BTNSHADOW ) );
    m_DefaultColors[GUI::COL_GRAYTEXT]            = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_GRAYTEXT ) );
    m_DefaultColors[GUI::COL_BTNTEXT]             = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_BTNTEXT ) );
    m_DefaultColors[GUI::COL_INACTIVECAPTIONTEXT] = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_INACTIVECAPTIONTEXT ) );
    m_DefaultColors[GUI::COL_BTNHIGHLIGHT]        = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_BTNHIGHLIGHT ) );
    m_DefaultColors[GUI::COL_3DDKSHADOW]          = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_3DDKSHADOW ) );

    m_DefaultColors[GUI::COL_3DLIGHT]                 = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_3DLIGHT ) );
    m_DefaultColors[GUI::COL_INFOTEXT]                = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_INFOTEXT ) );
    m_DefaultColors[GUI::COL_INFOBK]                  = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_INFOBK ) );
    m_DefaultColors[GUI::COL_HOTLIGHT]                = 0xff000000 | GFX::RGBToBGR( GetSysColor( 26 ) );
    m_DefaultColors[GUI::COL_GRADIENTACTIVECAPTION]   = 0xff000000 | GFX::RGBToBGR( GetSysColor( 27 ) );
    m_DefaultColors[GUI::COL_GRADIENTINACTIVECAPTION] = 0xff000000 | GFX::RGBToBGR( GetSysColor( 28 ) );
    m_DefaultColors[GUI::COL_MENUHILIGHT]             = 0xff000000 | GFX::RGBToBGR( GetSysColor( 29 ) );
    m_DefaultColors[GUI::COL_MENUBAR]                 = 0xff000000 | GFX::RGBToBGR( GetSysColor( 30 ) );
    m_DefaultColors[GUI::COL_HOTLIGHTTEXT]            = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_HIGHLIGHTTEXT ) );

    m_DefaultColors[GUI::COL_NC_AREA]                 = 0xffffffff;
    m_DefaultColors[GUI::COL_THIN_BORDER]             = 0xff000000;
    m_DefaultColors[GUI::COL_CURSOR]                  = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_WINDOWTEXT ) );
    m_DefaultColors[GUI::COL_WINDOWTEXT]              = 0xff000000 | GFX::RGBToBGR( GetSysColor( COL_WINDOWTEXT ) );

    m_DefaultColors[GUI::COL_HOTTIP_BACKGROUND]       = 0xc0ffffff;
    m_DefaultColors[GUI::COL_HOTTIP_TEXT]             = 0xff000000;

    if ( m_XMLGUIRoot.HasAttribute( "Color.SCROLLBAR" ) )
    {
      m_DefaultColors[GUI::COL_SCROLLBAR] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.SCROLLBAR" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.BACKGROUND" ) )
    {
      m_DefaultColors[GUI::COL_BACKGROUND] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.BACKGROUND" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.ACTIVECAPTION" ) )
    {
      m_DefaultColors[GUI::COL_ACTIVECAPTION] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.ACTIVECAPTION" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.INACTIVECAPTION" ) )
    {
      m_DefaultColors[GUI::COL_INACTIVECAPTION] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.INACTIVECAPTION" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.MENU" ) )
    {
      m_DefaultColors[GUI::COL_MENU] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.MENU" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.WINDOW" ) )
    {
      m_DefaultColors[GUI::COL_WINDOW] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.WINDOW" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.WINDOWFRAME" ) )
    {
      m_DefaultColors[GUI::COL_WINDOWFRAME] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.WINDOWFRAME" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.MENUTEXT" ) )
    {
      m_DefaultColors[GUI::COL_MENUTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.MENUTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.WINDOWTEXT" ) )
    {
      m_DefaultColors[GUI::COL_WINDOWTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.WINDOWTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.CAPTIONTEXT" ) )
    {
      m_DefaultColors[GUI::COL_CAPTIONTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.CAPTIONTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.ACTIVEBORDER" ) )
    {
      m_DefaultColors[GUI::COL_ACTIVEBORDER] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.ACTIVEBORDER" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.INACTIVEBORDER" ) )
    {
      m_DefaultColors[GUI::COL_INACTIVEBORDER] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.INACTIVEBORDER" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.APPWORKSPACE" ) )
    {
      m_DefaultColors[GUI::COL_APPWORKSPACE] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.APPWORKSPACE" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HIGHLIGHT" ) )
    {
      m_DefaultColors[GUI::COL_HIGHLIGHT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HIGHLIGHT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HIGHLIGHTTEXT" ) )
    {
      m_DefaultColors[GUI::COL_HIGHLIGHTTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HIGHLIGHTTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.BTNFACE" ) )
    {
      m_DefaultColors[GUI::COL_BTNFACE] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.BTNFACE" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.BTNSHADOW" ) )
    {
      m_DefaultColors[GUI::COL_BTNSHADOW] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.BTNSHADOW" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.GRAYTEXT" ) )
    {
      m_DefaultColors[GUI::COL_GRAYTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.GRAYTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.BTNTEXT" ) )
    {
      m_DefaultColors[GUI::COL_BTNTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.BTNTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.INACTIVECAPTIONTEXT" ) )
    {
      m_DefaultColors[GUI::COL_INACTIVECAPTIONTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.INACTIVECAPTIONTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.BTNHIGHLIGHT" ) )
    {
      m_DefaultColors[GUI::COL_BTNHIGHLIGHT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.BTNHIGHLIGHT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.THREEDDKSHADOW" ) )
    {
      m_DefaultColors[GUI::COL_3DDKSHADOW] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.THREEDDKSHADOW" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.THREEDDLIGHT" ) )
    {
      m_DefaultColors[GUI::COL_3DLIGHT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.THREEDDLIGHT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.INFOTEXT" ) )
    {
      m_DefaultColors[GUI::COL_INFOTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.INFOTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.INFOBK" ) )
    {
      m_DefaultColors[GUI::COL_INFOBK] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.INFOBK" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HOTLIGHT" ) )
    {
      m_DefaultColors[GUI::COL_HOTLIGHT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HOTLIGHT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.GRADIENTACTIVECAPTION" ) )
    {
      m_DefaultColors[GUI::COL_GRADIENTACTIVECAPTION] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.GRADIENTACTIVECAPTION" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.GRADIENTINACTIVECAPTION" ) )
    {
      m_DefaultColors[GUI::COL_GRADIENTINACTIVECAPTION] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.GRADIENTINACTIVECAPTION" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.MENUHILIGHT" ) )
    {
      m_DefaultColors[GUI::COL_MENUHILIGHT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.MENUHILIGHT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.MENUBAR" ) )
    {
      m_DefaultColors[GUI::COL_MENUBAR] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.MENUBAR" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.NC_AREA" ) )
    {
      m_DefaultColors[GUI::COL_NC_AREA] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.NC_AREA" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.THIN_BORDER" ) )
    {
      m_DefaultColors[GUI::COL_THIN_BORDER] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.THIN_BORDER" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.CURSOR" ) )
    {
      m_DefaultColors[GUI::COL_CURSOR] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.CURSOR" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HOTLIGHTTEXT" ) )
    {
      m_DefaultColors[GUI::COL_HOTLIGHTTEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HOTLIGHTTEXT" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HOTTIP_BACKGROUND" ) )
    {
      m_DefaultColors[GUI::COL_HOTTIP_BACKGROUND] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HOTTIP_BACKGROUND" ), 16 );
    }
    if ( m_XMLGUIRoot.HasAttribute( "Color.HOTTIP_TEXT" ) )
    {
      m_DefaultColors[GUI::COL_HOTTIP_TEXT] = GR::Convert::ToU32( m_XMLGUIRoot.Attribute( "Color.HOTTIP_TEXT" ), 16 );
    }

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Button", GUI::CTS_BUTTON );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ButtonMouseOver", GUI::CTS_BUTTON_MOUSEOVER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ButtonPushed", GUI::CTS_BUTTON_PUSHED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ButtonDisabled", GUI::CTS_BUTTON_DISABLED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ToggleMinus", GUI::CTS_TOGGLE_MINUS );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.TogglePlus", GUI::CTS_TOGGLE_PLUS );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowUp", GUI::CTS_ARROW_UP );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowUpMouseOver", GUI::CTS_ARROW_UP_MOUSEOVER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowUpPushed", GUI::CTS_ARROW_UP_PUSHED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowUpDisabled", GUI::CTS_ARROW_UP_DISABLED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowDown", GUI::CTS_ARROW_DOWN );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowDownMouseOver", GUI::CTS_ARROW_DOWN_MOUSEOVER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowDownPushed", GUI::CTS_ARROW_DOWN_PUSHED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowDownDisabled", GUI::CTS_ARROW_DOWN_DISABLED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowLeft", GUI::CTS_ARROW_LEFT );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowLeftMouseOver", GUI::CTS_ARROW_LEFT_MOUSEOVER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowLeftPushed", GUI::CTS_ARROW_LEFT_PUSHED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowLeftDisabled", GUI::CTS_ARROW_LEFT_DISABLED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowRight", GUI::CTS_ARROW_RIGHT );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowRightMouseOver", GUI::CTS_ARROW_RIGHT_MOUSEOVER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowRightPushed", GUI::CTS_ARROW_RIGHT_PUSHED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.ArrowRightDisabled", GUI::CTS_ARROW_RIGHT_DISABLED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.CheckboxChecked", GUI::CTS_CHECKBOX_CHECKED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.CheckboxUnchecked", GUI::CTS_CHECKBOX_UNCHECKED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.CheckboxCheck", GUI::CTS_CHECKBOX_CHECK );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.RadioChecked", GUI::CTS_RADIO_CHECKED );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.RadioUnchecked", GUI::CTS_RADIO_UNCHECKED );

    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider", GUI::CTS_SLIDER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.Top", GUI::CTS_SLIDER_TOP );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.VCenter", GUI::CTS_SLIDER_VCENTER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.Bottom", GUI::CTS_SLIDER_BOTTOM );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.Left", GUI::CTS_SLIDER_LEFT );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.HCenter", GUI::CTS_SLIDER_HCENTER );
    ApplyCustomSection( &m_XMLGUIRoot, "CustomSection.Slider.Right", GUI::CTS_SLIDER_RIGHT );

    for ( int i = 0; i < GUI::COL_LAST_ENTRY; ++i )
    {
      m_GUI.SysColor( (GUI::eColorIndex)i, m_DefaultColors[i] );
    }
  }



  void Loader::ApplyDefaultValues( GUI::Component* pComponent )
  {
    for ( int i = 0; i < GUI::COL_LAST_ENTRY; ++i )
    {
      if ( !pComponent->UseCustomColor( (GUI::eColorIndex)i ) )
      {
        pComponent->SetColor( (GUI::eColorIndex)i, m_DefaultColors[i], true );
      }
    }

    Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
    if ( pLoader == NULL )
    {
      dh::Log( "Loader::ApplyDefaultValues called, but no AssetLoader service found" );
      return;
    }

    tCustomSections::iterator   itCS( m_CustomSections.begin() );
    while ( itCS != m_CustomSections.end() )
    {
      const GR::String&     sectionName = itCS->second;

      Xtreme::Asset::XAsset* pSection = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, sectionName.c_str() );
      if ( pSection == NULL )
      {
        //dh::Log( "Loader::ApplyDefaultValues Section (%s) not found", sectionName.c_str() );
      }
      else
      {
        m_GUI.SetCustomTextureSection( itCS->first, *(XTextureSection*)pSection->Handle( "Section" ) );
        pComponent->SetCustomTextureSection( itCS->first, *(XTextureSection*)pSection->Handle( "Section" ) );
      }
      ++itCS;
    }
  }



  void Loader::OnVarEvent( LocalRegistry::LocalRegistryEvent Event, const GR::String& VarName )
  {
    if ( VarName == "GUI.Language" )
    {
      if ( m_pTable )
      {
        m_pTable->SetLangID( m_pVar->GetVarI( VarName ) );
        UpdateGUITextFromDB( &m_GUI );
      }
    }

    tTextDependenceDB::iterator   it( m_TextDependentControlsDB.find( VarName ) );
    if ( it != m_TextDependentControlsDB.end() )
    {
      tDependentControlsDB& DependentControls( it->second );

      tDependentControlsDB::iterator    itC( DependentControls.begin() );
      while ( itC != DependentControls.end() )
      {
        GUI::Component*   pComp = ( *itC );

        UpdateComponentText( pComp );

        ++itC;
      }
    }

    tTextDependence::iterator   it2( m_TextDependentControls.find( VarName ) );
    if ( it2 != m_TextDependentControls.end() )
    {
      tDependentControls& DependentControls( it2->second );

      tDependentControls::iterator    itC( DependentControls.begin() );
      while ( itC != DependentControls.end() )
      {
        UpdateStaticComponentText( itC->first, itC->second );

        ++itC;
      }
    }

  }



  void Loader::OnComponentCreatedFromXML( GR::Strings::XMLElement* pElement, GUI::Component* pComponent )
  {
    m_DelayedCreatedElements[NULL].push_back( std::make_pair( pElement, pComponent ) );
  }



  void Loader::OnComponentAdded( const GUI::OutputEvent& Event )
  {
    InitialiseCreatedElements( Event.pComponent );
  }



  void Loader::ReplaceDelayedParent( GUI::ComponentContainer* pContainer )
  {
    tMapDelayedCreatedElements::iterator    itC( m_DelayedCreatedElements.find( NULL ) );
    if ( itC == m_DelayedCreatedElements.end() )
    {
      return;
    }
    m_DelayedCreatedElements[pContainer] = m_DelayedCreatedElements[NULL];
    m_DelayedCreatedElements.erase( itC );
  }



  void Loader::InitialiseCreatedElements( GUI::ComponentContainer* pContainer )
  {
    tMapDelayedCreatedElements::iterator    itC( m_DelayedCreatedElements.find( pContainer ) );
    if ( itC == m_DelayedCreatedElements.end() )
    {
      return;
    }
    tCreatedElements&   createdElements( itC->second );

    tCreatedElements::iterator    it( createdElements.begin() );
    while ( it != createdElements.end() )
    {
      GR::Strings::XMLElement*    pXML = it->first;
      GUI::Component*                 pComponent = it->second;

      GUI::ComponentEvent             cEvent( CET_INIT_AFTER_LOAD, pComponent );
      cEvent.Value = (GR::ip)m_pTable;
      pComponent->ProcessEvent( cEvent );

      if ( pXML->HasAttribute( "HottipID" ) )
      {
        GR::u32     HottipID = GR::Convert::ToU32( pXML->Attribute( "HottipID" ) );
        if ( HottipID != -1 )
        {
          Component*   pHottip = (Component*)m_GUI.CreateObject( "Hottip" );
          if ( pHottip )
          {
            m_HottipControls[pComponent] = pHottip;

            pHottip->ProcessEvent( GUI::ComponentEvent( CET_ATTACH_COMPONENT, pComponent ) );

            GR::String     Caption = m_pTable->Entry( HottipID );
            pHottip->SetCaption( Caption );

            //dh::Log( "Add Hottip: %s", Caption.c_str() );
            m_GUI.Add( pHottip );

            pComponent->AddHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );
          }
        }
      }
      else if ( pXML->HasAttribute( "HotTipText" ) )
      {
        GR::String     text = pXML->Attribute( "HotTipText" );
        if ( !text.empty() )
        {
          Component*   pHottip = (Component*)m_GUI.CreateObject( "Hottip" );
          if ( pHottip )
          {
            m_HottipControls[pComponent] = pHottip;

            pHottip->ProcessEvent( GUI::ComponentEvent( CET_ATTACH_COMPONENT, pComponent ) );
            pHottip->SetCaption( text );
            m_GUI.Add( pHottip );

            pComponent->AddHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );
          }
        }
      }
      ApplyDefaultValues( pComponent );

      ++it;
    }
    m_DelayedCreatedElements.erase( itC );
  }



  Component* Loader::CreateBaseComponent( const GR::String& Name )
  {
    GR::Strings::XMLElement*    pXMLDoc = m_XMLGUI.FindByType( "GUIDocument" );
    if ( pXMLDoc == NULL )
    {
      GR::Strings::XML::iterator    it( m_XMLGUI.FirstChild() );
      while ( it != GR::Strings::XML::iterator() )
      {
        GR::Strings::XMLElement* pXMLElement( *it );

        if ( pXMLElement->Attribute( "Name" ) == Name )
        {
          m_GUI.AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          Component*    pComp = m_GUI.ComponentFromXML( pXMLElement );
          if ( pComp )
          {
            UpdateGUITextFromDB( pComp );
          }

          m_GUI.RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          if ( pComp )
          {
            pComp->AddHandler( OET_ADDED_TO_PARENT, fastdelegate::MakeDelegate( this, &Loader::OnComponentAdded ) );
          }
          ReplaceDelayedParent( pComp );
          return pComp;
        }

        it = it.next_sibling();
      }
      return NULL;
    }
    GR::Strings::XML::iterator    it( pXMLDoc->FirstChild() );
    
    while ( it != GR::Strings::XML::iterator() )
    {
      GR::Strings::XMLElement*    pXMLElement( *it );

      if ( ( pXMLElement->Type() == "Component" )
      &&   ( pXMLElement->Attribute( "Caption" ) == Name ) )
      {
        m_GUI.AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        Component*    pComp = m_GUI.ComponentFromXML( pXMLElement );
        if ( pComp )
        {
          UpdateGUITextFromDB( pComp );
        }

        m_GUI.RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        if ( pComp )
        {
          pComp->AddHandler( OET_ADDED_TO_PARENT, fastdelegate::MakeDelegate( this, &Loader::OnComponentAdded ) );
        }
        ReplaceDelayedParent( pComp );
        return pComp;
      }
      it = it.next_sibling();
    }
    return NULL;
  }



  Component* Loader::CreateBaseComponentByID( const GR::u32 ID )
  {
    GR::Strings::XMLElement*    pXMLDoc = m_XMLGUI.FindByType( "GUIDocument" );
    if ( pXMLDoc == NULL )
    {
      GR::Strings::XML::iterator    it( m_XMLGUI.FirstChild() );
      while ( it != GR::Strings::XML::iterator() )
      {
        GR::Strings::XMLElement* pXMLElement( *it );

        if ( GR::Convert::ToU32( pXMLElement->Attribute( "ID" ) ) == ID )
        {
          m_GUI.AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          Component*    pComp = m_GUI.ComponentFromXML( pXMLElement );
          if ( pComp )
          {
            UpdateGUITextFromDB( pComp );
          }

          m_GUI.RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          if ( pComp )
          {
            pComp->AddHandler( OET_ADDED_TO_PARENT, fastdelegate::MakeDelegate( this, &Loader::OnComponentAdded ) );
          }
          ReplaceDelayedParent( pComp );
          return pComp;
        }

        it = it.next_sibling();
      }
      return NULL;
    }
    GR::Strings::XML::iterator    it( pXMLDoc->FirstChild() );
    
    while ( it != GR::Strings::XML::iterator() )
    {
      GR::Strings::XMLElement*    pXMLElement( *it );

      if ( ( pXMLElement->Type() == "Component" )
      &&   ( GR::Convert::ToU32( pXMLElement->Attribute( "ID" ) ) == ID ) )
      {
        m_GUI.AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        Component*    pComp = m_GUI.ComponentFromXML( pXMLElement );
        if ( pComp )
        {
          UpdateGUITextFromDB( pComp );
        }

        m_GUI.RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        if ( pComp )
        {
          pComp->AddHandler( OET_ADDED_TO_PARENT, fastdelegate::MakeDelegate( this, &Loader::OnComponentAdded ) );
        }
        ReplaceDelayedParent( pComp );
        return pComp;
      }
      it = it.next_sibling();
    }
    return NULL;
  }



  void Loader::OnComponentDestroyed( const GUI::OutputEvent& Event )
  {
    //dh::Log( "Component (%s) destroyed", Event.pComponent->GetCaption().c_str() );
    // remove hottips if watched control is deleted
    tHottipControls::iterator   itHC( m_HottipControls.begin() );
    while ( itHC != m_HottipControls.end() )
    {
      if ( itHC->first == Event.pComponent )
      {
        m_GUI.Delete( itHC->second );
        m_HottipControls.erase( itHC++ );
      }
      else
      {
        ++itHC;
      }
    }

    tTextDependenceDB::iterator   it( m_TextDependentControlsDB.begin() );
    while ( it != m_TextDependentControlsDB.end() )
    {
      tDependentControlsDB& DependentControls( it->second );

      tDependentControlsDB::iterator    itC( DependentControls.begin() );
      while ( itC != DependentControls.end() )
      {
        GUI::Component*   pComp = ( *itC );

        if ( pComp == Event.pComponent )
        {
          itC = DependentControls.erase( itC );
          //dh::Log( "-removed from list (%d entries left)", DependentControls.size() );
        }
        else
        {
          ++itC;
        }
      }
      ++it;
    }

    tTextDependence::iterator   it2( m_TextDependentControls.begin() );
    while ( it2 != m_TextDependentControls.end() )
    {
      tDependentControls& DependentControls( it2->second );

      tDependentControls::iterator    itC( DependentControls.begin() );
      while ( itC != DependentControls.end() )
      {
        GUI::Component*   pComp = itC->first;

        if ( pComp == Event.pComponent )
        {
          DependentControls.erase( itC++ );
          //dh::Log( "-removed from list (%d entries left)", DependentControls.size() );
        }
        else
        {
          ++itC;
        }
      }
      ++it2;
    }
  }



  void Loader::UpdateGUITextFromDB( GUI::ComponentContainer* pContainer )
  {
    if ( ( pContainer == NULL )
    ||   ( m_pTable == NULL ) )
    {
      return;
    }
    tListComponents::iterator    it( pContainer->m_Components.begin() );
    while ( it != pContainer->m_Components.end() )
    {
      GUI::Component* pChildComponent( *it );

      if ( !pChildComponent->IsDestroyed() )
      {
        UpdateGUITextFromDB( pChildComponent );
      }

      ++it;
    }
  }



  GR::String Loader::TextFromDB( int Index )
  {
    if ( m_pTable == NULL )
    {
      return "GUILoader: No Table set";
    }
    return m_pTable->Entry( Index );
  }



  void Loader::UpdateComponentText( GUI::Component* pComponent, bool ModifyHandler )
  {
    GR::String     Caption = m_pTable->Entry( pComponent->Id() );

    if ( m_pVar )
    {
      size_t    BracketOpenPos = Caption.find( '[' );
      while ( BracketOpenPos != -1 )
      {
        size_t    BracketEndPos = Caption.find( ']', BracketOpenPos );
        if ( BracketEndPos == -1 )
        {
          break;
        }
        GR::String     Var = Caption.substr( BracketOpenPos + 1, BracketEndPos - BracketOpenPos - 1 );

        if ( ModifyHandler )
        {
          // store to update if a var changes
          pComponent->RemoveHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );
          pComponent->AddHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );

          m_TextDependentControlsDB[Var].remove( pComponent );
          m_TextDependentControlsDB[Var].push_back( pComponent );
          //dh::Log( "Register component %x for var %s", pComponent, Var.c_str() );
        }

        Var = m_pVar->GetVar( Var );

        GR::String     NewCaption;

        if ( BracketOpenPos > 0 )
        {
          NewCaption = Caption.substr( 0, BracketOpenPos );
        }
        NewCaption += Var;
        NewCaption += Caption.substr( BracketEndPos + 1 );
        BracketOpenPos = Caption.find( '[' );

        Caption = NewCaption;
      }
    }
    pComponent->SetCaption( Caption );
  }



  void Loader::UpdateStaticComponentText( GUI::Component* pComponent, const GR::String& NewCaption, bool ModifyHandler )
  {
    GR::String   Caption = NewCaption;
    if ( m_pVar )
    {
      size_t    BracketOpenPos = Caption.find( '[' );
      while ( BracketOpenPos != -1 )
      {
        size_t    BracketEndPos = Caption.find( ']', BracketOpenPos );
        if ( BracketEndPos == -1 )
        {
          break;
        }
        GR::String     Var = Caption.substr( BracketOpenPos + 1, BracketEndPos - BracketOpenPos - 1 );

        if ( ModifyHandler )
        {
          // store to update if a var changes
          pComponent->RemoveHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );
          pComponent->AddHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );

          tDependentControls&  Map( m_TextDependentControls[Var] );

          tDependentControls::iterator   itMap( Map.find( pComponent ) );
          if ( itMap != Map.end() )
          {
            Map.erase( itMap );
          }
          m_TextDependentControls[Var].insert( std::make_pair( pComponent, Caption ) );
          //dh::Log( "Register component %x for var %s", pComponent, Var.c_str() );
        }

        Var = m_pVar->GetVar( Var );

        GR::String     NewCaption;

        if ( BracketOpenPos > 0 )
        {
          NewCaption = Caption.substr( 0, BracketOpenPos );
        }
        NewCaption += Var;
        NewCaption += Caption.substr( BracketEndPos + 1 );
        BracketOpenPos = Caption.find( '[' );

        Caption = NewCaption;
      }
    }
    pComponent->SetCaption( Caption );
  }



  void Loader::UpdateGUITextFromDB( GUI::Component* pComponent )
  {
    if ( ( pComponent == NULL )
    ||   ( m_pTable == NULL ) )
    {
      return;
    }

    if ( pComponent->Flags() & GUI::COMPFT_CAPTION_FROM_DBASE )
    {
      UpdateComponentText( pComponent, true );
    }
    else
    {
      UpdateStaticComponentText( pComponent, pComponent->GetCaption(), true );
    }

    tListComponents::iterator    it( pComponent->m_Components.begin() );
    while ( it != pComponent->m_Components.end() )
    {
      GUI::Component* pChildComponent( *it );

      UpdateGUITextFromDB( pChildComponent );

      ++it;
    }
  }



};