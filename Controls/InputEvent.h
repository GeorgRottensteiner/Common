#ifndef INPUTEVENT_H
#define INPUTEVENT_H



#include <GR/GRTypes.h>



namespace GUI
{
  class Component;

  class ComponentEvent
  {
    public:

      GR::u32           Type,
                        MouseButtons;

      GR::tPoint        Position;
      GR::ip            Value;

      GR::u16           Character;

      Component*        pComponent;



      ComponentEvent( GR::u32 Type, Component* pComp = NULL ) :
        Type( Type ),
        MouseButtons( 0 ),
        Position( 0, 0 ),
        Value( 0 ),
        Character( 0 ),
        pComponent( pComp )
      {
      }

  };
}

#endif // INPUTEVENT_H



