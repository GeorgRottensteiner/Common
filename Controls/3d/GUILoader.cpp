#include "GUILoader.h"

#include <GR/Database/DatabaseTable.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include "GUIHottip.h"



namespace GUI
{


  Loader::Loader() :
    m_pTable( NULL ),
    m_pVar( NULL )
  {
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



  void Loader::StoreDefaultValues()
  {
    m_DefaultColors[GUI::COL_SCROLLBAR]           = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_SCROLLBAR ) );
    m_DefaultColors[GUI::COL_BACKGROUND]          = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_BACKGROUND ) );
    m_DefaultColors[GUI::COL_ACTIVECAPTION]       = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_ACTIVECAPTION ) );
    m_DefaultColors[GUI::COL_INACTIVECAPTION]     = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_INACTIVECAPTION ) );
    m_DefaultColors[GUI::COL_MENU]                = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_MENU ) );
    m_DefaultColors[GUI::COL_WINDOW]              = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_WINDOW ) );
    m_DefaultColors[GUI::COL_WINDOWFRAME]         = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_WINDOWFRAME ) );
    m_DefaultColors[GUI::COL_MENUTEXT]            = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_MENUTEXT ) );
    m_DefaultColors[GUI::COL_WINDOWTEXT]          = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_WINDOWTEXT ) );
    m_DefaultColors[GUI::COL_CAPTIONTEXT]         = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_CAPTIONTEXT ) );
    m_DefaultColors[GUI::COL_ACTIVEBORDER]        = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_ACTIVEBORDER ) );

    m_DefaultColors[GUI::COL_INACTIVEBORDER]      = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_INACTIVEBORDER ) );
    m_DefaultColors[GUI::COL_APPWORKSPACE]        = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_APPWORKSPACE ) );
    m_DefaultColors[GUI::COL_HIGHLIGHT]           = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_HIGHLIGHT ) );
    m_DefaultColors[GUI::COL_HIGHLIGHTTEXT]       = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
    m_DefaultColors[GUI::COL_BTNFACE]             = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_BTNFACE ) );
    m_DefaultColors[GUI::COL_BTNSHADOW]           = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_BTNSHADOW ) );
    m_DefaultColors[GUI::COL_GRAYTEXT]            = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_GRAYTEXT ) );
    m_DefaultColors[GUI::COL_BTNTEXT]             = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_BTNTEXT ) );
    m_DefaultColors[GUI::COL_INACTIVECAPTIONTEXT] = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_INACTIVECAPTIONTEXT ) );
    m_DefaultColors[GUI::COL_BTNHIGHLIGHT]        = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_BTNHIGHLIGHT ) );
    m_DefaultColors[GUI::COL_3DDKSHADOW]          = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_3DDKSHADOW ) );

    m_DefaultColors[GUI::COL_3DLIGHT]                 = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_3DLIGHT ) );
    m_DefaultColors[GUI::COL_INFOTEXT]                = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_INFOTEXT ) );
    m_DefaultColors[GUI::COL_INFOBK]                  = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_INFOBK ) );
    m_DefaultColors[GUI::COL_HOTLIGHT]                = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( 26 ) );
    m_DefaultColors[GUI::COL_GRADIENTACTIVECAPTION]   = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( 27 ) );
    m_DefaultColors[GUI::COL_GRADIENTINACTIVECAPTION] = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( 28 ) );
    m_DefaultColors[GUI::COL_MENUHILIGHT]             = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( 29 ) );
    m_DefaultColors[GUI::COL_MENUBAR]                 = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( 30 ) );
    m_DefaultColors[GUI::COL_HOTLIGHTTEXT]            = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );

    m_DefaultColors[GUI::COL_NC_AREA]                 = 0xffffffff;
    m_DefaultColors[GUI::COL_THIN_BORDER]             = 0xff000000;
    m_DefaultColors[GUI::COL_CURSOR]                  = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_WINDOWTEXT ) );
    m_DefaultColors[GUI::COL_WINDOWTEXT]              = 0xff000000 | CGFXHelper::RGBToBGR( ::GetSysColor( COLOR_WINDOWTEXT ) );

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
  }



  void Loader::OnVarEvent( LocalRegistry::LocalRegistryEvent Event, const GR::String& VarName )
  {
    if ( VarName == "GUI.Language" )
    {
      if ( m_pTable )
      {
        m_pTable->SetLangID( m_pVar->GetVarI( VarName ) );
        UpdateGUITextFromDB( &GUIComponentDisplayer::Instance() );
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
      GUI::Component*             pComponent = it->second;

      if ( pXML->HasAttribute( "HottipID" ) )
      {
        GR::u32     HottipID = GR::Convert::ToU32( pXML->Attribute( "HottipID" ) );
        if ( HottipID != -1 )
        {
          CGUIHottip*   pHottip = (CGUIHottip*)GUIComponentDisplayer::Instance().CreateObject( "Hottip" );
          if ( pHottip )
          {
            m_HottipControls[pComponent] = pHottip;
            pHottip->Attach( pComponent );

            GR::String     Caption = m_pTable->Entry( HottipID );
            pHottip->SetCaption( Caption );

            //dh::Log( "Add Hottip: %s", Caption.c_str() );
            GUIComponentDisplayer::Instance().Add( pHottip );

            pComponent->AddHandler( OET_DESTROY, fastdelegate::MakeDelegate( this, &Loader::OnComponentDestroyed ) );
          }
        }
      }
      ApplyDefaultValues( pComponent );

      ++it;
    }
    m_DelayedCreatedElements.erase( itC );
  }



  GUIComponent* Loader::CreateBaseComponent( const GR::String& Name )
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
          GUIComponentDisplayer::Instance().AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          GUIComponent*    pComp = (GUIComponent*)GUIComponentDisplayer::Instance().ComponentFromXML( pXMLElement );
          if ( pComp )
          {
            UpdateGUITextFromDB( pComp );
          }

          GUIComponentDisplayer::Instance().RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
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
        GUIComponentDisplayer::Instance().AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        GUIComponent*    pComp = (GUIComponent*)GUIComponentDisplayer::Instance().ComponentFromXML( pXMLElement );
        if ( pComp )
        {
          UpdateGUITextFromDB( pComp );
        }

        GUIComponentDisplayer::Instance().RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
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



  GUIComponent* Loader::CreateBaseComponentByID( const GR::u32 ID )
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
          GUIComponentDisplayer::Instance().AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
          GUIComponent*    pComp = (GUIComponent*)GUIComponentDisplayer::Instance().ComponentFromXML( pXMLElement );
          if ( pComp )
          {
            UpdateGUITextFromDB( pComp );
          }

          GUIComponentDisplayer::Instance().RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
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
        GUIComponentDisplayer::Instance().AddElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
        GUIComponent*    pComp = (GUIComponent*)GUIComponentDisplayer::Instance().ComponentFromXML( pXMLElement );
        if ( pComp )
        {
          UpdateGUITextFromDB( pComp );
        }

        GUIComponentDisplayer::Instance().RemoveElementCreatedFromXMLListener( fastdelegate::MakeDelegate( this, &Loader::OnComponentCreatedFromXML ) );
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
    //dh::Log( "Component (%s) destroyed", Event.m_pComponent->GetCaption().c_str() );
    // remove hottips if watched control is deleted
    tHottipControls::iterator   itHC( m_HottipControls.begin() );
    while ( itHC != m_HottipControls.end() )
    {
      if ( itHC->first == Event.pComponent )
      {
        GUIComponentDisplayer::Instance().Delete( itHC->second );
        itHC = m_HottipControls.erase( itHC );
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
          //dh::Log( "-removed from list (%d entries left)", DependantControls.size() );
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
          itC = DependentControls.erase( itC );
          //dh::Log( "-removed from list (%d entries left)", DependantControls.size() );
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
    tListComponents::iterator    it( pContainer->m_listComponents.begin() );
    while ( it != pContainer->m_listComponents.end() )
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
          NewCaption = Caption.substr( 0, BracketOpenPos - 1 );
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

    tListComponents::iterator    it( pComponent->m_listComponents.begin() );
    while ( it != pComponent->m_listComponents.end() )
    {
      GUI::Component* pChildComponent( *it );

      UpdateGUITextFromDB( pChildComponent );

      ++it;
    }
  }



};