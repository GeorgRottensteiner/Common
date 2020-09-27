#ifndef GR_GAMEBASE_FRAMEWORK_FRAMEEVENT_H
#define GR_GAMEBASE_FRAMEWORK_FRAMEEVENT_H

#include <GR/GRTypes.h>
#include <string>


namespace GR
{

  namespace Gamebase
  {

    struct tXFrameEvent
    {
      enum eEventType
      {
        ET_INVALID = 0,
        ET_RENDERER_SWITCHED,
        ET_SOUND_SWITCHED,
        ET_INPUT_SWITCHED,
        ET_DISPLAY_MODE_CHANGED,
        ET_USER,
        ET_CONSOLE_COMMAND,
        ET_CONSOLE_TOGGLED,
        ET_SHUTDOWN,
        ET_MUSIC_PLAYER_SWITCHED,
        ET_PRE_WINDOW_MODE_CHANGE,
        ET_WINDOW_MODE_CHANGED
      };

      eEventType      m_Type;

      GR::up          m_Param1,
                      m_Param2;
      GR::String      m_Text;

      tXFrameEvent( eEventType Type, GR::up Param1 = 0, GR::up Param2 = 0 ) :
        m_Type( Type ),
        m_Param1( Param1 ),
        m_Param2( Param2 )
      {
      }
      tXFrameEvent( eEventType Type, const GR::String& Param, GR::up Param1 = 0, GR::up Param2 = 0 ) :
        m_Type( Type ),
        m_Param1( Param1 ),
        m_Param2( Param2 ),
        m_Text( Param )
      {
      }
    };
  }
}


#endif // GR_GAMEBASE_FRAMEWORK_FRAMEEVENT_H