#ifndef OUTPUTEVENT_H
#define OUTPUTEVENT_H



#include <GR/GRTypes.h>

#include <list>



enum OutputEventType
{
  OET_INVALID               = 0,
  OET_MOUSE_IN              = 0x00001000,
  OET_MOUSE_OUT             = 0x00001001,
  OET_MOUSE_UP              = 0x00001002,
  OET_MOUSE_DOWN            = 0x00001003,
  OET_MOUSE_UPDATE          = 0x00001004,
  OET_MOUSE_RUP             = 0x00001005,
  OET_MOUSE_RDOWN           = 0x00001006,
  OET_MOUSE_WHEEL           = 0x00001007,   // Param1 = x, Param2 = y, Param3 = buttons, Param4 = 1/-1

  OET_KEY_DOWN              = 0x00001100,
  OET_KEY_UP                = 0x00001101,
  OET_KEY_PRESSED           = 0x00001102,

  OET_ITEM_HOVER            = 0x00001200,   // user moved mouse over an item

  OET_OWNER_DRAW            = 0x00001300,   // GUI::OwnerDrawInfo in Event.m_Param2
  OET_OWNER_DRAW_ITEM       = 0x00001301,   // GUI::OwnerDrawInfo in Event.m_Param2

  OET_FOCUS_LOOSE           = 0x00002000,
  OET_FOCUS_GET             = 0x00002001,

  OET_CAPTURE_BEGIN         = 0x00002010,
  OET_CAPTURE_END           = 0x00002011,

  OET_ADDED_TO_PARENT       = 0x00002020,
  OET_REMOVED_FROM_PARENT   = 0x00002021,
  OET_ADDED_CHILD           = 0x00002022,
  OET_REMOVED_CHILD         = 0x00002033,

  OET_DRAG_CONTENT_START    = 0x00002030,
  OET_DRAG_CONTENT_CANCEL   = 0x00002031,
  OET_DRAG_CONTENT_DROP     = 0x00002032,
  OET_DRAG_CONTENT_QUERY    = 0x00002033,

  OET_BUTTON_PUSHED         = 0x00003000,
  OET_BUTTON_DOWN           = 0x00003001,
  OET_BUTTON_UP             = 0x00003002,

  OET_SLIDER_BEGIN_DRAG     = 0x00004000,
  OET_SLIDER_END_DRAG       = 0x00004001,
  OET_SLIDER_SCROLLED       = 0x00004002,

  OET_EDIT_CHANGE           = 0x00005000,
  OET_EDIT_ENTER_PRESSED    = 0x00005001,

  OET_SCROLLBAR_SCROLLED    = 0x00006000,

  OET_LISTBOX_ITEM_SELECTED = 0x00007000,
  OET_LISTBOX_ITEM_DBLCLK   = 0x00007001,
  OET_LISTBOX_ITEM_KEY_ACTIVATE = 0x00007002,
  OET_LISTBOX_ITEM_CLICK    = 0x00007003,
  OET_LISTBOX_HEADER_CLICK  = 0x00007010,

  OET_TREE_ITEM_SELECTED    = 0x00008000,
  OET_TREE_ITEM_DBLCLK      = 0x00008001,
  OET_TREE_ITEM_EXPAND      = 0x00008002,
  OET_TREE_ITEM_COLLAPSE    = 0x00008003,
  OET_TREE_ITEM_ACTIVATE    = 0x00008004,       // activate via key (enter)

  OET_LABEL_DBLCLK          = 0x00008100,       // double click

  OET_SET_POSITION          = 0x00009000,
  OET_DESTROY               = 0x00009100,
  OET_CLOSE                 = 0x00009101,       // Close mit Close-Code

  OET_COMBO_SELCHANGE       = 0x0000a000,       // param1 = item index, param2 = item data

  OET_PROGRESS_POSCHANGE    = 0x0000b000,
};



namespace GUI
{
  class Component;

  class OutputEvent
  {
    public:

      OutputEventType   Type;

      GR::u32           Id;

      GR::up            Param1,
                        Param2,
                        Param3,
                        Param4;

      Component*        pComponent;


      OutputEvent( OutputEventType Type, GR::u32 Id, Component* pComponent = NULL, 
                   GR::up Param1 = 0, GR::up Param2 = 0, GR::up Param3 = 0, GR::up Param4 = 0 ) :
        Type( Type ),
        Id( Id ),
        Param1( Param1 ),
        Param2( Param2 ),
        Param3( Param3 ),
        Param4( Param4 ),
        pComponent( pComponent )
      {
      }

      OutputEvent( OutputEvent* pEvent ) :
        Type( pEvent->Type ),
        Id( pEvent->Id ),
        Param1( pEvent->Param1 ),
        Param2( pEvent->Param2 ),
        Param3( pEvent->Param3 ),
        Param4( pEvent->Param4 ),
        pComponent( pEvent->pComponent )
      {
      }


  };

  typedef std::list<OutputEvent*>             tListOutputEvents;
  typedef std::list<OutputEvent*>::iterator   itListOutputEvents;
}


#endif // OUTPUTEVENT_H



