#ifndef GUI_COMPONENT_LOADER_H
#define GUI_COMPONENT_LOADER_H



#include "GUIComponent.h"
#include "GUIComponentDisplayer.h"

#include <String/XML.h>

#include <Interface/IIOStream.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <string>
#include <vector>


namespace GR
{
  namespace Database
  {
    class Table;
  };
};

namespace GUI
{

  class Loader : public GR::Service::Service
  {

    protected:

      typedef std::list<GUI::Component*>                      tDependentControlsDB;
      typedef std::map<GUI::Component*,GR::String>           tDependentControls;
      typedef std::map<GUI::Component*,GUI::Component*>           tHottipControls;

      typedef std::map<GR::String,tDependentControlsDB>  tTextDependenceDB;

      typedef std::map<GR::String,tDependentControls>    tTextDependence;

      typedef std::list<std::pair<GR::Strings::XMLElement*,GUI::Component*> >   tCreatedElements;
      typedef std::map<GUI::ComponentContainer*,tCreatedElements>      tMapDelayedCreatedElements;

      tTextDependence                 m_TextDependentControls;
      tTextDependenceDB               m_TextDependentControlsDB;

      //tCreatedElements                m_CreatedElements;

      tMapDelayedCreatedElements      m_DelayedCreatedElements;

      tHottipControls                 m_HottipControls;

      GR::Strings::XML                m_XMLGUI;

      GR::Strings::XMLElement         m_XMLGUIRoot;

      GR::Database::Table*            m_pTable;

      LocalRegistry*                  m_pVar;

      GR::u32                         m_DefaultColors[GUI::COL_LAST_ENTRY];



      void                            UpdateComponentText( GUI::Component* pComponent, bool ModifyHandler = false );
      void                            UpdateStaticComponentText( GUI::Component* pComponent, const GR::String& OrigText, bool ModifyHandler = false );

      void                            OnComponentDestroyed( const GUI::OutputEvent& Event );
      void                            OnVarEvent( LocalRegistry::LocalRegistryEvent Event, const GR::String& VarName );
      void                            OnComponentCreatedFromXML( GR::Strings::XMLElement* pElement, GUI::Component* pComponent );
      void                            InitialiseCreatedElements( GUI::ComponentContainer* pContainer );
      void                            ReplaceDelayedParent( GUI::ComponentContainer* pContainer );

      void                            StoreDefaultValues();
      void                            ApplyDefaultValues( GUI::Component* pComponent );


    public:


      Loader();

      bool                            LoadFromXML( IIOStream& ioIn );
      bool                            Initialize( IIOStream& ioIn, GR::Database::Table* pTable = NULL, LocalRegistry* pLocalRegistry = NULL );
      bool                            Initialize( GR::Strings::XMLElement* pGUIRootAsset, GR::Database::Table* pTable, LocalRegistry* pLocalRegistry );

      GUIComponent*                   CreateBaseComponent( const GR::String& Name );
      GUIComponent*                   CreateBaseComponentByID( const GR::u32 ID );

      void                            UpdateGUITextFromDB( GUI::Component* pComponent );
      void                            UpdateGUITextFromDB( GUI::ComponentContainer* pContainer );

      GR::String                     TextFromDB( int Index );

      void                            OnComponentAdded( const GUI::OutputEvent& Event );

  };

};

#endif // GUI_COMPONENT_LOADER_H



