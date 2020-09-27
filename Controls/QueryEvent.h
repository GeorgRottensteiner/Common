#ifndef GUI_QUERY_EVENT_H
#define GUI_QUERY_EVENT_H



#include <GR/GRTypes.h>



namespace GUI
{
  struct QueryEventType
  {
    enum Value
    {
      QET_INVALID           = 0,

      QET_CAN_DRAG_CONTENT  = 0x00001000        // if ResultValue == 0 -> drag allowed,   == 1 -> drag not allowed
    };
  };

  class QueryEvent
  {
    public:

      QueryEventType::Value     Type;
      size_t                    ItemIndex;

      GR::up                    ResultValue;
      GR::up                    ResultValue2;

      Component*                pComponent;



      QueryEvent( QueryEventType::Value Type, Component* pComponent = NULL, size_t ItemIndex = 0 ) :
        Type( Type ),
        ItemIndex( ItemIndex ),
        ResultValue( 0 ),
        ResultValue2( 0 ),
        pComponent( pComponent )
      {
      }
  };

}



#endif // GUI_QUERY_EVENT_H



