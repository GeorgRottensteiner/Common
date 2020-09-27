#ifndef GUI_COMPONENT_LOADER_H
#define GUI_COMPONENT_LOADER_H



#include "ComponentDisplayerBase.h"

#include <String/XML.h>

#include <Interface/IGUILoader.h>
#include <Interface/IIOStream.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <string>
#include <vector>


namespace GR
{
  namespace Database
  {
    class Table;
  }
}

namespace GUI
{

  class Loader : public GR::Service::ServiceImpl<IGUILoader>// GR::Service::Service,
                 //public IGUILoader
  {

    protected:

      typedef std::list<Component*>                      tDependentControlsDB;
      typedef std::map<Component*,GR::String>           tDependentControls;
      typedef std::map<Component*,Component*>           tHottipControls;

      typedef std::map<GR::String,tDependentControlsDB>  tTextDependenceDB;

      typedef std::map<GR::String,tDependentControls>    tTextDependence;

      typedef std::list<std::pair<GR::Strings::XMLElement*,Component*> >  tCreatedElements;
      typedef std::map<ComponentContainer*,tCreatedElements>              tMapDelayedCreatedElements;

      typedef std::map<GUI::eCustomTextureSections, GR::String>       tCustomSections;

      tTextDependence                 m_TextDependentControls;
      tTextDependenceDB               m_TextDependentControlsDB;     

      tCustomSections                 m_CustomSections;

      //tCreatedElements                m_CreatedElements;

      tMapDelayedCreatedElements      m_DelayedCreatedElements;

      tHottipControls                 m_HottipControls;

      GR::Strings::XML                m_XMLGUI;

      GR::Strings::XMLElement         m_XMLGUIRoot;

      GR::Database::Table*            m_pTable;

      LocalRegistry*                  m_pVar;

      GR::u32                         m_DefaultColors[GUI::COL_LAST_ENTRY];

      ComponentDisplayerBase&     m_GUI;



      void                            UpdateComponentText( Component* pComponent, bool ModifyHandler = false );
      void                            UpdateStaticComponentText( Component* pComponent, const GR::String& OrigText, bool ModifyHandler = false );

      void                            OnComponentDestroyed( const GUI::OutputEvent& Event );
      void                            OnVarEvent( LocalRegistry::LocalRegistryEvent Event, const GR::String& VarName );
      void                            OnComponentCreatedFromXML( GR::Strings::XMLElement* pElement, Component* pComponent );
      void                            InitialiseCreatedElements( GUI::ComponentContainer* pContainer );
      void                            ReplaceDelayedParent( GUI::ComponentContainer* pContainer );

      void                            StoreDefaultValues();
      void                            ApplyDefaultValues( GUI::Component* pComponent );

      // set global default custom sections
      void                            ApplyCustomSection( GR::Strings::XMLElement* pElement, 
                                                          const GR::String& AttributeName,
                                                          GUI::eCustomTextureSections CustomSection );

      GR::u32                         GetSysColor( GR::u32 ColorIndex );


    public:


      Loader( ComponentDisplayerBase& GUI );
      virtual ~Loader();

      bool                            LoadFromXML( IIOStream& ioIn );
      bool                            Initialize( IIOStream& ioIn, GR::Database::Table* pTable = NULL, LocalRegistry* pLocalRegistry = NULL );
      bool                            Initialize( GR::Strings::XMLElement* pGUIRootAsset, GR::Database::Table* pTable, LocalRegistry* pLocalRegistry );

      Component*                      CreateBaseComponent( const GR::String& Name );
      Component*                      CreateBaseComponentByID( const GR::u32 ID );

      virtual void                    UpdateGUITextFromDB( GUI::Component* pComponent );
      virtual void                    UpdateGUITextFromDB( GUI::ComponentContainer* pContainer );

      GR::String                      TextFromDB( int Index );

      void                            OnComponentAdded( const GUI::OutputEvent& Event );

  };

};

#endif // GUI_COMPONENT_LOADER_H



