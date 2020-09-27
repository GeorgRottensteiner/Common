#ifndef I_GUI_COMPONENT_LOADER_H
#define I_GUI_COMPONENT_LOADER_H

#include <Interface/IService.h>



namespace GUI
{
  class Component;
  class ComponentContainer;

  class IGUILoader : public GR::IService
  {
    public:

      virtual ~IGUILoader() {};

      virtual void    UpdateGUITextFromDB( GUI::Component* pComponent ) = 0;
      virtual void    UpdateGUITextFromDB( GUI::ComponentContainer* pContainer ) = 0;
  };

}

#endif // I_GUI_COMPONENT_LOADER_H



