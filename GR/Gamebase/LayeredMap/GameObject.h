#ifndef JR_ENGINE_GAME_OBJECT_H
#define JR_ENGINE_GAME_OBJECT_H

#include "ObjectLayer.h"

#include <Interface/IAnimationManager.h>

#include <Game/MovementPath.h>



namespace GR
{
  namespace Gamebase
  {
    class JREngine;
    class GameObject;

    namespace ObjectEventType
    {
      enum Value
      {
        INVALID,
        CREATED,
        KILLED,                // might end up with die, could morph too
        DIE,                   // object is going to be removed
        BLOCKED_LEFT,
        BLOCKED_W = BLOCKED_LEFT,
        BLOCKED_RIGHT,
        BLOCKED_E = BLOCKED_RIGHT,
        BLOCKED_N,
        BLOCKED_S,
        LAND,
        JUMP,
        START_FALLING,
        AWAKEN,
        TO_SLEEP,
        LAND_ON_OBJECT,
        OBJECT_LANDED_ON_ME,
        GET_OFF_OBJECT,
        OBJECT_GOT_OFF_ME,
        CUSTOM,
        MOVED,
        DELETED,                // object is being deleted
        WARP,
        CUSTOM_DRAW,
        MOVED_L,
        MOVED_R,
        MOVED_U,
        MOVED_D,
        COLLIDE
      };
    }

    class ObjectEvent
    {
      public:

        ObjectEventType::Value  Type;
        GameObject*             pOtherObject;
        GR::ip                  Param;
        GR::tPoint              PosOrDelta;


        ObjectEvent( ObjectEventType::Value Type = ObjectEventType::INVALID, GameObject* pOtherObj = NULL, GR::ip Param = 0 ) :
          Type( Type ),
          pOtherObject( pOtherObj ),
          Param( Param )
        { 
        }

        ObjectEvent( ObjectEventType::Value Type, const GR::tPoint& PosOrDelta, GR::ip Param = 0 ) :
          Type( Type ),
          pOtherObject( NULL ),
          Param( Param ),
          PosOrDelta( PosOrDelta )
        {
        }
    };

    namespace ProcessingFlags
    {
      enum Value
      {
        NONE                  = 0x00000000,
        DELETE_ME             = 0x00000001,   // replaces RemoveMe
        SEND_TO_SLEEP         = 0x00000002,   // remove from awake object list
        AWAKENED              = 0x00000004,
        VANISH_IF_OFF_SCREEN  = 0x00000008,
        CUSTOM_RENDER         = 0x00000010,   // calls custom draw event
        ABORT_MOVE            = 0x00000020,   // set in event to jump out of MoveObject/Fixed method
        NO_UPDATE             = 0x00000040,   // skip updates for this object
        NO_ANIMATION          = 0x00000080    // pause animation
      };
    }


    class GameObject : public GR::Gamebase::LayerObject
    {
      private:

        GR::tRect                   m_Bounds;


      protected:


        void                        HandleGravity( GR::Gamebase::JREngine& JREngine );
        void                        HandleJump( GR::Gamebase::JREngine& JREngine, GR::f32 ElapsedTime );



      public:

        GR::tVector                 m_Delta;

        GR::Gamebase::JREngine*     m_pEngine;

        // uses ProcessingFlags::Value
        GR::u32                     m_ProcessingFlags;

        bool                        m_CustomRender;

        GR::tFPoint                 FractPos;
        GR::tPoint                  DisplayOffset;
        int                         ZOrder;
        GR::f32                     Angle;
        int                         Type;         // to use instead of template for faster checking, requires "Type" in ParameterTable
        int                         FallPos;
        int                         FallHeight;

        // duration it takes to reach the peak of the jump arc
        GR::f32                     JumpDuration;
        int                         JumpArcHeight;
        int                         JumpStartPos;
        GR::f32                     JumpPos;

        GR::f32                     LifeTime;

        GameObject*                 m_pSpawnAncestor;


        GR::String                  Section;
        GR::String                  Animation;
        tAnimationPos               AnimPos;

        GameObject*                 m_pObjectBelow;

        std::list<GameObject*>      m_ObjectsCarried;

        GR::Game::Path::tPathPos    m_MovementPathPos;



        GameObject();
        virtual ~GameObject();


        GR::tRect                   Bounds();
        void                        Bounds( const GR::tRect& Bounds );


        virtual void                Update( GR::Gamebase::JREngine& JREngine, const GR::f32 ElapsedTime, GR::up UserData );
        virtual void                UpdateFixed( GR::Gamebase::JREngine& JREngine, const GR::f32 ElapsedTime, GR::up UserData );

        virtual void                OnObjectEvent( JREngine& JREngine, const ObjectEvent& Event );

        void                        SetSection( const GR::String& Section );
        void                        SetAnimation( const GR::String& Section );

        // completely kill/delete object
        void                        DeleteMe();
        void                        KillMe();

        // remove from awake object list, but NOT delete!
        void                        SendToSleep();

        void                        Jump( int JumpHeight );
        // use forced jump, no checks for onground, etc.
        void                        ForceJump( int JumpHeight );

        void                        AbortMove();

    };
  }
}

#endif // JR_ENGINE_GAME_OBJECT_H
